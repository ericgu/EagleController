#include "Command.h"

#include "Strip.h"

#include "AnimationCommands.h"

#include "IAnimation.h"
#include "AnimationBlendTo.h"
#include "AnimationAlternate.h"
#include "AnimationIndividual.h"
#include "AnimationColorRotate.h"
#include "AnimationFlashDecay.h"
#include "AnimationSetChunk.h"
#include "AnimationRandomFlash.h"

class PixelHandler
{
  private:
    Strip* _pStrip;
    PersistentStorage* _pPersistentStorage;
    int _pixelPin;

    const int AnimationCount = 7;
    IAnimation** _pAnimations;
    IAnimation* _pCurrentAnimation;
    bool _pixelCountUpdated;
    AnimationCommands _animationCommands;

  public:
    PixelHandler(PersistentStorage* pPersistentStorage, int pixelPin) : 
        _pPersistentStorage(pPersistentStorage),
        _pixelPin(pixelPin)
    { 
      _pStrip = 0;
      _pixelCountUpdated = false;
    }

    Strip* GetStrip()
    {
      return _pStrip;
    }

    int GetPixelCount()
    {
      return _pPersistentStorage->_ledCount;
    }

    void CreateAnimations()
    {
      _pAnimations = new IAnimation*[AnimationCount];

      _pAnimations[0] = new AnimationBlendTo(_pStrip);
      _pAnimations[1] = new AnimationAlternate(_pStrip);
      _pAnimations[2] = new AnimationIndividual(_pStrip);
      _pAnimations[3] = new AnimationColorRotate(_pStrip);
      _pAnimations[4] = new AnimationFlashDecay(_pStrip);
      _pAnimations[5] = new AnimationSetChunk(_pStrip);
      _pAnimations[6] = new AnimationRandomFlash(_pStrip);
    }

    void ClearStrip()
    {
      _pStrip->Clear();
    }

    void InitStrip()
    {
      Serial.print("InitString"); Serial.println(_pPersistentStorage->_ledCount);
      _pStrip = new Strip(_pixelPin, _pPersistentStorage->_ledCount);
      _pStrip->Init();
    }

    void Init()
    {
      InitStrip();
      CreateAnimations();

      SetUnconnectedAnimation();

      //_pCurrentAnimation->Update();
    }

    void SetUnconnectedAnimation()
    {
      Serial.println("Red alternate");
      Command command("alt 100,000,000,000,000,000,250");
      ProcessMessage(command);
    }

    void SetAccessPointAnimation()
    {
      Serial.println("Yellow alternate");
      Command command("alt 150,150,000,000,000,000,250");
      ProcessMessage(command);
    }

    void SetProvisionedAnimation()
    {
      Serial.print("Attempting restart using: ");
      Serial.println(_pPersistentStorage->_storedAnimation);
      _animationCommands.LoadFromString(_pPersistentStorage->_storedAnimation);
      Serial.println("Loaded");
    }

    String GetCurrentCommand()
    {
      return _animationCommands.SaveToString();
    }

    void ProcessMessage(Command& command)
    {
      if (command.StartsWith("s"))   // save the current state to flash
      {
        String savedAnimation = _animationCommands.SaveToString();
        
        if (_pPersistentStorage->SetStoredAnimation(savedAnimation))
        {
          Serial.println("Saving animation");
          Serial.println(savedAnimation);
          _pPersistentStorage->Save();
        }
      }
      else if (command.StartsWith("n"))   // Update the LED count
      {
        _pPersistentStorage->_ledCount = command._values[0];
        _pixelCountUpdated = true;
        _pPersistentStorage->Save();
        _pStrip->Clear();
        ESP.restart();
      }
      else if (command.StartsWith("reset"))
      {
        _pPersistentStorage->Reset();
        _pStrip->Clear();
        ESP.restart();        
      }
      else
      {
        _animationCommands.Add(command);
      }
    }

    void CheckForCommandUpdate()
    {        
      Command* pCommand = _animationCommands.GetNextMessage();
      if (!pCommand)
      {
        return;
      }
      
      for (int i = 0; i < AnimationCount; i++)
      {
        if (_pAnimations[i]->ProcessMessage(pCommand))
        {
          _pCurrentAnimation = _pAnimations[i];
          Serial.print("Switched to: ");
          Serial.print(_pCurrentAnimation->getName());
          Serial.print(" -> ( ");
          Serial.print(pCommand->_period);
          Serial.print(") ");
          Serial.println(pCommand->_message);
            
          return;
        }
      }

      Serial.print("Unrecognized: ");
      Serial.println(pCommand->_message);
    }

    void Update()
    {
      CheckForCommandUpdate();
      
      _pCurrentAnimation->Update();
    }
};

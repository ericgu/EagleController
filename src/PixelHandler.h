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
#include "AnimationChaser.h"

class PixelHandler
{
  private:
    Strip* _pStrip;
    PersistentStorage* _pPersistentStorage;
    int _pixelPin;

    const int AnimationCount = 8;
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
      _animationCommands.SetAnimation(_pPersistentStorage->_storedAnimation);
      _pCurrentAnimation = 0;
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
      _pAnimations[7] = new AnimationChaser(_pStrip);
    }

    void ClearStrip()
    {
      _pStrip->Clear();
    }

    void InitStrip()
    {
      Serial.print("InitStrip: "); Serial.println(_pPersistentStorage->_ledCount);
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
      SetDefaultAnimation();   
    }

    void SetAccessPointAnimation()
    {
      Serial.println("Yellow alternate");
      SetDefaultAnimation();   
    }

    void SetDefaultAnimation()
    {
      _animationCommands.SetAnimation("$200$rgbx200,0,0,200$200$rgbx0,0,0,200$200$rgbx0,200,0,200$200$rgbx0,0,0,200$200$rgbx0,0,200,200$200$rgbx0,0,0,200");
    }

    void SetProvisionedAnimation()
    {
      Serial.print("Attempting restart using: ");
      Serial.println(_pPersistentStorage->_storedAnimation);
      if (strlen(_pPersistentStorage->_storedAnimation) != 0)
      {
        ProcessMessage(_pPersistentStorage->_storedAnimation);
      }
      Serial.println("Loaded");
    }

    String GetCurrentCommand()
    {
      return _pPersistentStorage->_storedAnimation;
    }

    void ProcessMessage(String commandString)
    {
      if (commandString.startsWith("s"))   // save the current state to flash
      {
        Serial.println("Saving animation");
        _pPersistentStorage->Save();
      }
      else if (commandString.startsWith("n"))   // Update the LED count
      {
        Command command(commandString, -1);
        _pPersistentStorage->_ledCount = command._values[0];
        _pixelCountUpdated = true;
        _pPersistentStorage->Save();
        _pStrip->Clear();
        ESP.restart();
      }
      else if (commandString.startsWith("reset"))
      {
        _pPersistentStorage->Reset();
        _pStrip->Clear();
        ESP.restart();        
      }
      else
      {
        Serial.print("Update: "); Serial.println(commandString);

        _pPersistentStorage->SetStoredAnimation(commandString);
        _animationCommands.SetAnimation(_pPersistentStorage->_storedAnimation);
      }
    }

    void CheckForCommandUpdate()
    {        
      Command command = _animationCommands.GetNextMessage();

      if (command._period == -1)
      {
        return;
      }

      //Serial.print("CheckForCommandUpdate: "); Serial.println(command._message);

      for (int i = 0; i < AnimationCount; i++)
      {
        if (_pAnimations[i]->ProcessMessage(&command))
        {
          _pCurrentAnimation = _pAnimations[i];
          Serial.print("Switched to: ");
          Serial.print(_pCurrentAnimation->getName());
          Serial.print(" -> ( ");
          Serial.print(command._period);
          Serial.print(") ");
          Serial.println(command._message);
            
          return;
        }
      }

      Serial.print("Unrecognized: ");
      Serial.println(command._message);
    }

    void Update()
    {
      CheckForCommandUpdate();
      
      if (_pCurrentAnimation)
      {
        _pCurrentAnimation->Update();
      }
    }
};

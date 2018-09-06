#include "Tokenizer.h"

class AnimationCommands
{
  private:
  
    Command* _pCommands;
    int _currentCommand;
    int _commandCount;
    int _cycle;


    void LoadFromString(String storedAnimationFormat)
    {
      _commandCount = 0;
      _pCommands = 0;
      _currentCommand = -1;
      _cycle = 0;

      if (storedAnimationFormat.startsWith("$"))
      {
        _commandCount = 0;
        const char* pString = storedAnimationFormat.c_str();
        while (*pString != '\0')
        {
          if (*pString == '$')
          {
            _commandCount++;
          }
          pString++;
        }
        _commandCount /= 2;
        Allocate(_commandCount);
        
        Tokenizer tokenizer(storedAnimationFormat, '$');

        int segment = 0;
        while (!tokenizer.GetFinished())
        {
          int period = tokenizer.GetToken().toInt();
          String message = tokenizer.GetToken();

          //Serial.print("Command: "); Serial.println(message);
          *(_pCommands + segment) = Command(message, period);
          segment++;
        }
      }
      else if (storedAnimationFormat != NULL)
      {
        Allocate(1);

        *_pCommands = Command(storedAnimationFormat.c_str(), 1000000);
      }
    }

    void Allocate(int commandCount)
    {
      _commandCount = commandCount;
      _pCommands = new Command[_commandCount];
    }

  public:
    AnimationCommands(char* commands)
    {
      LoadFromString(String(commands));
    }

    AnimationCommands(String commands)
    {
      LoadFromString(commands);
    }

    AnimationCommands(const AnimationCommands &ob)
    {
      LoadFromString(ob.SaveToString());
    }

    ~AnimationCommands()
    {
      if (_pCommands)
      {
        delete _pCommands;
      }
    }

    String SaveToString() const
    {
      String s;

      Command* pSegments = _pCommands;

      for (int i = 0; i < _commandCount; i++)
      {
        s.concat("$");
        char buffer[7];
        itoa(pSegments->_period, buffer, 10);
        s.concat(buffer);
        s.concat("$");
        s.concat(pSegments->_message);
        pSegments++;
      }

      return s;
    }

    void DumpSegments()
    {
      Serial.println("Commands: ");
      for (int i = 0; i < _commandCount; i++)
      {
        Command *pTemp = _pCommands + i;
        Serial.print(i); Serial.print(": "); Serial.print(pTemp->_period); Serial.print(" "); Serial.println(pTemp->_message);
      }
    }

    Command GetNextMessage()
    {
      Serial.println("GetNextMessage");
      Serial.println(_cycle);
      Serial.println(_currentCommand);
      Serial.println("GetNextMessage");

      if (_cycle <= 0 && _pCommands)
      {
        _currentCommand = (_currentCommand + 1) % _commandCount;

        Command* pCommand = _pCommands + _currentCommand;

        _cycle = pCommand->_period;

        if (_commandCount != 0)
        {
          pCommand->Dump();
          return *pCommand;
        }
      }
      _cycle--;

      return Command(0, -1);
    }

    int Count()
    {
      return _commandCount;
    }

    Command GetSegment(int index)
    {
      if (index >= _commandCount)
      {
        return 0;
      }

      return *(_pCommands + index);
    }

    //=operator function with deep copy.
    void operator=(const AnimationCommands &ob)
    {
      LoadFromString(ob.SaveToString());
    }
};


#include "Tokenizer.h"

class AnimationCommands
{
  private:
  
    Command* _pSegments;
    int _currentSegment;
    int _segmentCount;
    int _segmentAllocated;
    int _cycle;

  public:

    AnimationCommands()
    {
      _currentSegment = 0;
      _segmentCount = 0;
      _cycle = 0;

      _segmentAllocated = 10;
      _pSegments = new Command[_segmentAllocated];
    }

    void LoadFromString(String storedAnimationFormat)
    {
      if (storedAnimationFormat.startsWith("$"))
      {
        _segmentCount = 0;
        
        Tokenizer tokenizer(storedAnimationFormat, '$');

        while (!tokenizer.GetFinished())
        {
          int period = tokenizer.GetToken().toInt();
          String message = tokenizer.GetToken();

          //Serial.print("Command: "); Serial.println(message);
          Add(Command(message, period));
        }
      }
      else
      {
        Command command(storedAnimationFormat.c_str(), -1);
        Add(command);
      }
    }

    String SaveToString()
    {
      String s;

      Command* pSegments = _pSegments;

      for (int i = 0; i < _segmentCount; i++)
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

    void Add(Command& command)
    {
      if (command._period < 0)
      {
        _segmentCount = 0;
        _currentSegment = 0;
        _cycle = 0;
        command._period = 1000000;
      }

      if (command._message.startsWith("$"))
      {
        LoadFromString(command._message);
      }
      else
      {
          // resize if necessary
          
        if (_segmentCount == _segmentAllocated)
        {
          Command* pNew = new Command[_segmentAllocated * 2];

          for (int i = 0; i < _segmentCount; i++)
          {
            *(pNew + i) = *(_pSegments + i);
          }

          delete _pSegments;
          _pSegments = pNew;
          _segmentAllocated = _segmentAllocated * 2;
        }

        *(_pSegments + _segmentCount) = command;
        _segmentCount++;

        //DumpSegments();        

          // force switch to the new one on the next cycle. 
        _currentSegment = _segmentCount - 1;
        _cycle = 0;
      }
    }

    void DumpSegments()
    {
      Serial.println("Segments: ");
      for (int i = 0; i < _segmentCount; i++)
      {
        Command *pTemp = _pSegments + i;
        Serial.print(i); Serial.print(": "); Serial.print(pTemp->_period); Serial.print(" "); Serial.println(pTemp->_message);
      }
    }

    Command* GetNextMessage()
    {
      //Serial.println("GetNextMessage");
      //Serial.println(_cycle);
      //Serial.println(_currentSegment);
      //Serial.println("GetNextMessage");

      if (_cycle <= 0)
      {
        _currentSegment = (_currentSegment + 1) % _segmentCount;

        Command* pSegment = _pSegments + _currentSegment;

        _cycle = pSegment->_period;

        if (_segmentCount != 0)
        {
          pSegment->Dump();
          return pSegment;
        }
      }
      _cycle--;

      return 0;
    }

    int Count()
    {
      return _segmentCount;
    }

    Command GetSegment(int index)
    {
      if (index >= _segmentCount)
      {
        return 0;
      }

      return *(_pSegments + index);
    }
};


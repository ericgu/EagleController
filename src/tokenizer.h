class Tokenizer
{
  String _string;
  int _start;
  char _separator;
  bool _finished;
  
public:
  Tokenizer(String string, char separator)
  {
    _finished = false;
    _string = string;
    _start = 0;
    _separator = separator;
    if (_string.charAt(0) == '$')
    {
      _start = 1;
    }
  }

  String GetToken()
  {
    int end = _start + 1;

    char c;
    while (1)
    {
      c = _string.charAt(end);
      if (c == '$' || c == 0)
      {
        break;
      }
      end++;
    }

    String token = _string.substring(_start, end);
    if (c == 0)
    {
      _finished = true;
    }
    else if (c == '$')
    {
      _start = end + 1;
    }

    return token;
  }

  bool GetFinished()
  {
    return _finished;
  }
};


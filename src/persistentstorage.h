#include "EEPROM.h"

class PersistentStorageV1001
{
  public:

    int  _version;
    int _size;
    char _ssid[33];
    char _password[65];
    char _hostName[33];
    char _ledCount;
    char _storedAnimation[3956];

	void Save()
	{
		_size = sizeof(PersistentStorageV1001);

		EEPROM.begin(4096);
		byte* pData = (byte*) this;
		for (int i = 0; i < _size; i++)
		{
			EEPROM.write(i, *pData);
			pData++;
		}

		EEPROM.commit();
		Serial.println("Saved configuration");
	}
};

class PersistentStorage
{
  public:

    int  _version;
    int _size;
    char _ssid[33];
    char _password[65];
    char _hostName[33];
    short _ledCount;
    char _reserved[512];
    char _storedAnimation[3439];

    PersistentStorage()
    {
      memset(this, 0, sizeof(PersistentStorage));
	    _version = 1002;
	  }

    void Save()
    {
      _size = sizeof(PersistentStorage);
      
      EEPROM.begin(4096);
      byte* pData = (byte*) this;
      for (int i = 0; i < _size; i++)
      {
        EEPROM.write(i, *pData);
        pData++;
      }
      
      EEPROM.commit();
      Serial.println("Saved configuration");
    }

    void Load()
    {
      Serial.println("Load");
      EEPROM.begin(4096);

      byte* pStart = (byte*) this;
      byte* pData = pStart;
      int size = sizeof(PersistentStorage);
      for (int i = 0; i < size; i++)
      {
        *pData = EEPROM.read(i);
        pData++;
      }      

      if (_version != 1002)
      {
        PersistentStorageV1001* pOld = (PersistentStorageV1001*) this;

        String oldStoredAnimation = pOld->_storedAnimation;
        int oldLedCount = pOld->_ledCount;
        
        _version = 1002;
        _ledCount = oldLedCount;
		
        strncpy(_storedAnimation, oldStoredAnimation.c_str(), sizeof(_storedAnimation));
        Save(); // save on upgrade...
      }

      if (_ledCount > 1000)
      {
        _ledCount = 33;
      }

      if (*_storedAnimation == 255)
      {
        strcpy(_storedAnimation, "colx500,180");
      }

      Serial.println("Loaded configuration");
      Serial.println(_version);
      Serial.println(_size);
      Serial.println(_hostName);
      Serial.println(_ssid);
      Serial.println(_password);
      Serial.println(_storedAnimation);
      Serial.println((int) _ledCount);
    }

    void Reset()
    {
      strcpy(_hostName, "");
      strcpy(_ssid, "");
      strcpy(_password, "");
      strcpy(_storedAnimation, "colx500,180");
      _ledCount = 33;
      Save();
    }

    void SsidSet(String ssid)
    {
      strcpy(_ssid, ssid.c_str());
    }

    String SsidGet()
    {
      return String(_ssid);
    }
  
    void PasswordSet(String password)
    {
      strcpy(_password, password.c_str());
    }

    String PasswordGet()
    {
      return String(_password);
    }
  
    void HostNameSet(String hostName)
    {
      strcpy(_hostName, hostName.c_str());
    }

    String HostNameGet()
    {
      return String(_hostName);
    }

    bool SetStoredAnimation(String storedAnimation)
    {
      if (storedAnimation.length() < sizeof(_storedAnimation))
      {
        storedAnimation.toCharArray(_storedAnimation, sizeof(_storedAnimation));
        return true;
      }

      return false;
    }
};



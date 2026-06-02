

#include <Preferences.h>

class DFpreferences : public Preferences {
public:
  bool hasBytes(const char* key);
  bool hasString(const char* key);
};
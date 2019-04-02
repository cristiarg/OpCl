#pragma once

class CWSAInitializer final {
public:
  CWSAInitializer();
  ~CWSAInitializer();
  void mp_Cleanup();
private:
  bool mv_bInitialized;
};

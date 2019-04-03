#pragma once

#include <string>

enum class EMessageType {
    eInvalid

  , eValue
  , eOperator
  , eResult
  , eOK
  , eError
  , eQuit
};

enum class EOperation {
    eNone
  , eAdd
  , eDiff
  , eMulti
  , eDiv
  , eExp
  , eMod
};

class CMessageDecoder final {
public:
  CMessageDecoder(const std::string& m);
  CMessageDecoder(const CMessageDecoder&  ) = delete;
  CMessageDecoder(      CMessageDecoder&& ) = delete;
        
  CMessageDecoder operator=(const CMessageDecoder&  ) = delete;
  CMessageDecoder operator=(      CMessageDecoder&& ) = delete;

  bool isValid() const;
  EMessageType getType() const;
  int getValue() const;
  EOperation getOperation() const;
  std::string getErrorMessage() const;

private:
  void decode();

private:
  const std::string mess;

  bool valid;
  EMessageType type;
  int value;
  EOperation operation;
  std::string errorMessage;
};



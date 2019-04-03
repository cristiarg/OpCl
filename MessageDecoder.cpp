#include "MessageDecoder.h"

#include <sstream>
#include <cassert>

CMessageDecoder::CMessageDecoder(const std::string& m)
  : mess{ m }
  , valid{ false }
  , type{ EMessageType::eInvalid }
  , value{ 0 }
  , operation{ EOperation::eNone }
{
  decode();
}

bool CMessageDecoder::isValid() const
{
  return valid;
}

EMessageType CMessageDecoder::getType() const
{
  return type;
}

int CMessageDecoder::getValue() const
{
  return value;
}

EOperation CMessageDecoder::getOperation() const
{
  return operation;
}

std::string CMessageDecoder::getErrorMessage() const
{
  return errorMessage;
}

void CMessageDecoder::decode()
{
  if (mess.size() > 0) {
    std::stringstream ss { mess };
    char c;
    ss >> c;
    valid = true;
    switch (c) {
      case 'V': {
        type = EMessageType::eValue;
        ss >> value;
        break;
      }
      case 'O': {
        type = EMessageType::eOperator;
        char op; 
        ss >> std::skipws >> op;
      
        switch (op) {
          case '+':
            operation = EOperation::eAdd;
            break;
          case '-':
            operation = EOperation::eDiff;
            break;
          case '*':
            operation = EOperation::eMulti;
            break;
          case '/':
            operation = EOperation::eDiv;
            break;
          case '^':
            operation = EOperation::eExp;
            break;
          case '%':
            operation = EOperation::eMod;
            break;
          default:
            valid = false;
        }
        break;
      }
      case 'R': {
        type = EMessageType::eResult;
        ss >> value;
        break;
      }
      case 'K': {
        type = EMessageType::eOK;
        assert(mess.size() == 1);
        break;
      }
      case 'E': {
        type = EMessageType::eError;
        std::ostringstream oss;
        ss >> oss.rdbuf();
        errorMessage = oss.str();
        break;
      }
      case 'Q': {
        type = EMessageType::eQuit;
        assert(mess.size() == 1);
        break;
      }
      default: {
        valid = false;
      }
    }
  }
  else {
    valid = false;
  }
}



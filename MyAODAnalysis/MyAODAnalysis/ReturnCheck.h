#ifndef CxAODTools_ReturnCheck_H
#define CxAODTools_ReturnCheck_H

#define TOOL_CHECK( CONTEXT, EXP )                            \
  if (!EXP.isSuccess()) {                                     \
    Error( CONTEXT, "Failed to execute: %s. Exiting.", #EXP); \
    return EL::StatusCode::FAILURE;                           \
  }

#define EL_CHECK( CONTEXT, EXP )                              \
  if (EXP !=  EL::StatusCode::SUCCESS) {                      \
    Error( CONTEXT, "Failed to execute: %s. Exiting.", #EXP); \
    return EL::StatusCode::FAILURE;                           \
  }

#define CP_CHECK( CONTEXT, EXP, DEBUG )                                               \
  if (EXP ==  CP::CorrectionCode::Error) {                                            \
    Error( CONTEXT, "Failed to execute: %s. Exiting.", #EXP);                         \
    return EL::StatusCode::FAILURE;                                                   \
  }/*                                                                                    \ */
  /* if(DEBUG) {                                                                         \ */
  /*   if(EXP ==  CP::CorrectionCode::OutOfValidityRange) {                              \ */
  /*     Warning( CONTEXT, "%s returned CP::CorrectionCode::OutOfValidityRange.", #EXP); \ */
  /*   }                                                                                 \ */
  /*   if (EXP ==  CP::SystematicCode::Unsupported) {                                    \ */
  /*     Warning( CONTEXT, "%s returned CP::SystematicCode::Unsupported.", #EXP);        \ */
  /*   }                                                                                 \ */
  /* } */
#endif

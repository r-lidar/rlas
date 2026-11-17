/*
===============================================================================

  FILE:  rlaslasmessage.cpp

  CONTENTS:

    LASlib message handler for rlas. The bundled LASlib default handler prints nothing (its
    fprintf(stderr) calls are commented out for CRAN). This handler prints LASlib messages to the R
    console with REprintf(), exactly as the former patched default handler did.

    It calls R, so it must only run on the main thread. LazBlocks mutes its worker threads
    (las_mute_thread), whose messages never reach the handler.

===============================================================================
*/

#include "lasmessage.hpp"

#include <R_ext/Print.h>

#include <string>

// Defined in LASzip/lasmessage.cpp: indents the continuation lines of a multiline message
void format_message(std::string& messsage, unsigned multiline_ident, bool append_trailing_lf);

static void rlas_las_message_handler(LAS_MESSAGE_TYPE type, const char* msg, void* user_data)
{
  (void)user_data;

  std::string prefix;
  std::string message(msg);

  switch (type)
  {
  case LAS_DEBUG:
  case LAS_VERY_VERBOSE:
  case LAS_VERBOSE:
  case LAS_INFO:
  case LAS_QUIET:
    break;
  case LAS_WARNING:
    prefix = "WARNING: ";
    break;
  case LAS_SERIOUS_WARNING:
    prefix = "SERIOUS WARNING: ";
    break;
  case LAS_ERROR:
    prefix = "ERROR: ";
    break;
  case LAS_FATAL_ERROR:
    prefix = "FATAL ERROR: ";
    break;
  }

  if (!prefix.empty())
  {
    format_message(message, (unsigned)prefix.size(), true);
    REprintf("%s", prefix.c_str());
    REprintf("%s", message.c_str());
  }
  else
  {
    REprintf("%s\n", message.c_str());
  }
}

// Registered when the shared library is loaded, before any LASlib call. LASlib's message_handler is
// constant-initialized, so it is already set when this dynamic initializer runs.
static const bool rlas_las_message_handler_registered = []() {
  set_las_message_handler(&rlas_las_message_handler);
  return true;
}();

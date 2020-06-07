#include "net_interface.h"
#include "debug_interface.h"
#include "command_parser.h"
#include "wifi_debug_ostream.h"
#include <vector>
#include <algorithm>

namespace CommandParser
{

  enum class HasArg {
    Yes,
    No
  };


/// @brief The Template for a bee-focuser command
///
/// 
class CommandTemplate
{
  public:

  const std::string inputCommand;
  CommandParser::Command outputCommand;
  const HasArg hasArg;
};

const std::vector<CommandTemplate> commandTemplates =
{
  { "ping",       Command::Ping,       HasArg::No   },
  { "motora",     Command::SetMotorA,  HasArg::Yes  },
}; 

/// @brief Process an integer argument
///
/// Read an integer argument from a string in a way that's guaranteed
/// not to allocate memory 
///
/// @param[in] string - The string
/// @param[in] pos    - The start position in the string.  i.e., if pos=5
///   we'll look for the number at string element 5.
/// @return           - The result.  Currently 0 if there's no number.
///
int process_int( const std::string& string,  size_t pos )
{
  size_t end = string.length();
  if ( pos > end )
    return 0;

  const bool negative = (string[pos] == '-');
  if ( negative ) ++pos;

  int result = 0;
  for ( size_t iter = pos; iter != end; iter++ ) {
    char current = string[ iter ];
    if ( current >= '0' && current <= '9' )
      result = result * 10 + ( current - '0' );
    else
      break;
  }
  return negative ? -result : result;
}

const CommandPacket checkForCommands( 
	DebugInterface& serialLog, 
	NetInterface& wifi  )
{
	CommandPacket result;
  
  WifiDebugOstream log( &serialLog, &wifi );

  // Read the first line of the request.  

  static std::string command;
  bool dataReady = wifi.getString( command );
  if ( !dataReady )
  {
    return result;
  }

  std::transform( command.begin(), command.end(), command.begin(), ::tolower);

  log << "Got: " << command << "\n";

  for ( const CommandTemplate& ct : commandTemplates )
  {
    if ( command.find(ct.inputCommand ) == 0 )
    {
      result.command = ct.outputCommand;
      if ( ct.hasArg == HasArg::Yes )
      {
        result.optionalArg =  process_int( command,  ct.inputCommand.length()+1  );
      } 
      return result;
    }
  } 
  return result;

}


}

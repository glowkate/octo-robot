#include <memory>
#include "net_interface.h"
#include "net_esp8266.h"
#include "wifi_ostream.h"
#include "wifi_debug_ostream.h"

WifiInterfaceEthernet::WifiInterfaceEthernet(
  std::shared_ptr<DebugInterface> logArg
) 
  : log{ logArg }
{
  defaultConnection = std::make_shared< WifiConnectionEthernet>( logArg );
  delay(10);
  (*log) << "Init Wifi\n";

  // Connect to WiFi network
  (*log) << "Connecting to " << ssid << "\n";

  // Disable Wifi Persistence.  It's not needed and wears the flash memory.
  // Kudos Erik H. Bakke for pointing this point.
  WiFi.persistent( false );
  WiFi.mode( WIFI_STA );
  WiFi.hostname( hostname );
  WiFi.begin(ssid, password);
   
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    (*log) << ".";
  }
  (*log) << "\n";
  (*log) << "WiFi Connected\n";
   
  // Start the server
  m_server.begin();
  (*log) << "Server started\n";

  // Print the IP address
  UrbanRobot::IpAddress adr;
  auto dsIP = WiFi.localIP();
  for ( int i = 0; i < 4; ++ i )
    adr[i] = dsIP[i];
  (*log) << "Telnet to this address to connect: " << adr << " " << tcp_port << "\n";

  //wifi_set_sleep_type(LIGHT_SLEEP_T);
  reset();
}

Time::TimeUS WifiInterfaceEthernet::execute()
{
  if ( m_server.hasClient() )
  {  
    (*log) << "New client connecting\n";
   
    defaultConnection->initConnection( m_server );
  }
  return Time::TimeUS{ 10 * 1000 };
}

void WifiInterfaceEthernet::reset(void)
{
  defaultConnection->reset();
}

// ==========================================================================

void WifiConnectionEthernet::initConnection( WiFiServer &server )
{
  if ( m_connectedClient )
  {
      (*this) << "# New Client and no free slots - Dropping Your Connection.\n";
      m_connectedClient.stop();
  }
  m_connectedClient = server.available();
  m_connectedClient.setNoDelay( false );
  m_connectedClient.setSync( false );
  (*this) << "# Urban Octo Robot is ready for commands\n"; 
}

void WifiConnectionEthernet::writePushImpl( NetPipe& pipe )
{
  pipe.getChar();
}

Time::TimeUS WifiConnectionEthernet::execute()
{
  size_t maxWrite = m_connectedClient.availableForWrite();
  if ( maxWrite ) 
  {
    NetPipe::Buffer outBuf = writeBuffer.readView( maxWrite );
    if ( outBuf.second ) 
    {
      size_t written = m_connectedClient.write( outBuf.first, outBuf.second );
      writeBuffer.readAdvance( written );
    }
  }
  int numAvailable = m_connectedClient.available();
  if ( numAvailable ) 
  {
    NetPipe::Buffer inBuff = readBuffer.writeView( numAvailable );
    int numRead = m_connectedClient.read( (uint8_t*) inBuff.first, inBuff.second );
    readBuffer.writeAdvance( numRead );
  }

  return Time::TimeUS( 1000 );
}



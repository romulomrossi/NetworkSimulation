//Config files
#define LINK_CONFIG_PATH "Config/link.config"
#define ROUTER_CONFIG_PATH "Config/router.config"

//Log file
#define LOG_FILE_PATH "Log/log"

//Error messages
#define ERROR_ROUTER_NOT_FOUND "The router id %d was not founded at router.config!\n"
#define ERROR_NULL_ROUTER_ID "The router id must be passed! Try again.\n"
#define ERROR_TOO_LONG_MESSAGE "Maximum message size exceded, it could not be sent\n"
#define ERROR_FULL_BUFFER "Could not to send message. Full buffer, try again later.\n"
#define ERROR_UNREACHEABLE_ROUTER "Sorry, but the router %d could not be reach. \n"
#define ERROR_SOCKET_CREATE "Error while create router socket. Could not to continue.\n"
#define ERROR_MESSAGE_RETRY "Error while sending package, retrying\n"
#define ERROR_MESSAGE_ABORT "It's not possible to send package\n"

//Succsess messages
#define SUCCESS_MESSAGE_SENT "Message sent!\n"

//Separators
#define LEFT_SEPARATOR "------------------------| "
#define RIGHT_SEPARATOR " |------------------------\n"

//Infinite
#define INF 112345678

//Router buffer size
#define BUFFER_SIZE 10

//Message max size
#define MESSAGE_SIZE 100

//Number of tries to send package 
#define TRIES 5

//Timeout in package send in milliseconds
#define TIMEOUT 2
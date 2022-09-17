#ifndef CONFIG_PARSER
#define CONFIG_PARSER

class ConfigParser
{
public:
	ConfigParser(void);
	~ConfigParser(void);
	bool parseConfig(const char *config_file_path = "");
};

#endif
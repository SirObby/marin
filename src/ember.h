#if !defined(_H_EMBER)
#define _H_EMBER

#include <dpp/dpp.h>
#include <string>

class ember
{
private:
    dpp::embed em;
public:
    ember();
    ~ember();
    ember& add_description(std::string desc);
    ember& set_title(std::string title);
    ember& add_field(std::string name, std::string value, bool inl);
    ember& set_image(std::string img);
    ember& set_thumbnail(std::string img);
    ember& set_url(std::string url);
    dpp::embed return_emb();
};


#endif // _H_EMBER

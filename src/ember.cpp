#include "ember.h"

ember::ember()
{
    this->em = dpp::embed().set_footer("Marin ♡", "https://images-ext-2.discordapp.net/external/yuWQyYHNamIH8o5RXB9CWATQPtT54N_UZBfcJA5-AXk/https/i.pinimg.com/originals/a2/25/42/a22542523b0ecb54f1abdc6df3373336.gif").set_color(std::stol("e7c4ca", nullptr, 16));
}

ember::~ember()
{
}

ember& ember::add_description(std::string desc) {
    this->em.set_description(desc);
    return *this;
}
ember& ember::set_title(std::string title) {
    this->em.set_title(title);
    return *this;
}
ember& ember::add_field(std::string name, std::string value, bool inl) {
    this->em.add_field(name, value, inl);
    return *this;
}
ember& ember::set_image(std::string img) {
    this->em.set_image(img);
    return *this;
}
ember& ember::set_thumbnail(std::string img) {
    this->em.set_thumbnail(img);
    return *this;
}
ember& ember::set_url(std::string url) {
    this->em.set_url(url);
    return *this;
}
dpp::embed ember::return_emb() {
    return this->em;
};
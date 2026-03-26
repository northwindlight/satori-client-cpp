#include "Satori.h"
 
namespace satori {
    namespace event {
        template<typename T>
        static void opt(const nlohmann::json& j, const std::string& key, std::optional<T>& out)
        {
            if (j.contains(key) && !j[key].is_null())
                out = j[key].get<T>();
        }

        static void tryAttr(tinyxml2::XMLElement* elem, const char* name, std::optional<std::string>& out) {
            if (auto v = elem->Attribute(name)) out = v;
        }

        void from_json(const nlohmann::json& j, User& u)
        {
            j.at("id").get_to(u.id);
            opt(j, "name",   u.name);
            opt(j, "nick",   u.nick);
            opt(j, "avatar", u.avatar);
            opt(j, "is_bot", u.is_bot);
        }

        void from_json(const nlohmann::json& j, ChannelType& t)
        {
            t = static_cast<ChannelType>(j.get<int>());
        }

        void from_json(const nlohmann::json& j, Channel& c)
        {
            j.at("id").get_to(c.id);
            j.at("type").get_to(c.type);
            opt(j, "name",      c.name);
            opt(j, "parent_id", c.parent_id);
        }

        void from_json(const nlohmann::json& j, Guild& g)
        {
            j.at("id").get_to(g.id);
            opt(j, "name",   g.name);
            opt(j, "avatar", g.avatar);
        }

        void from_json(const nlohmann::json& j, GuildRole& r)
        {
            j.at("id").get_to(r.id);
            opt(j, "name", r.name);
        }

        void from_json(const nlohmann::json& j, GuildMember& m)
        {
            opt(j, "user",      m.user);
            opt(j, "nick",      m.nick);
            opt(j, "avatar",    m.avatar);
            opt(j, "joined_at", m.joined_at);
            if (j.contains("roles") && j["roles"].is_array())
                m.role = j["roles"].get<std::vector<GuildRole>>();
        }

        void from_json(const nlohmann::json& j, Message& m)
        {
            j.at("id").get_to(m.id);
            j.at("content").get_to(m.content);
            opt(j, "channel",    m.channel);
            opt(j, "guild",      m.guild);
            opt(j, "member",     m.member);
            opt(j, "user",       m.user);
            opt(j, "created_at", m.created_at);
            opt(j, "updated_at", m.updated_at);
        }

        void from_json(const nlohmann::json& j, LoginStatus& s)
        {
            s = static_cast<LoginStatus>(j.get<int>());
        }

        void from_json(const nlohmann::json& j, Login& l)
        {
            j.at("sn").get_to(l.sn);
            j.at("status").get_to(l.status);
            j.at("adapter").get_to(l.adapter);
            opt(j, "platform", l.platform);
            opt(j, "user",     l.user);
            if (j.contains("features") && j["features"].is_array())
                l.features = j["features"].get<std::vector<std::string>>();
        }

        void from_json(const nlohmann::json& j, Argv& a)
        {
            j.at("name").get_to(a.name);
            if (j.contains("arguments") && j["arguments"].is_array())
                a.arguments = j["arguments"].get<std::vector<nlohmann::json>>();
            if (j.contains("options"))
                a.options = j["options"];
        }

        void from_json(const nlohmann::json& j, Button& b)
        {
            j.at("id").get_to(b.id);
        }

        void from_json(const nlohmann::json& j, Event& e)
        {
            j.at("sn").get_to(e.sn);
            j.at("type").get_to(e.type);
            j.at("timestamp").get_to(e.timestamp);
            opt(j, "login",    e.login);
            opt(j, "argv",     e.argv);
            opt(j, "button",   e.button);
            opt(j, "channel",  e.channel);
            opt(j, "guild",    e.guild);
            opt(j, "member",   e.member);
            opt(j, "message",  e.message);
            opt(j, "operator", e.operator_);  //"operator"
            opt(j, "role",     e.role);
            opt(j, "user",     e.user);
            if (j.contains("referrer") && !j["referrer"].is_null())
                e.referrer = j["referrer"];
        }
    }
    namespace element {
        Elements parse(const std::string& content) {
            tinyxml2::XMLDocument doc;
            std::string xml = "<root>" + content + "</root>";
            if (doc.Parse(xml.c_str()) != tinyxml2::XML_SUCCESS)
                return { content };

            Elements result;
            auto* root = doc.FirstChildElement("root");

            auto tryAttr = [](tinyxml2::XMLElement* el, const char* key, std::optional<std::string>& out) {
                if (auto v = el->Attribute(key)) out = v;
            };

            for (auto* node = root->FirstChild(); node; node = node->NextSibling()) {
                if (auto* text = node->ToText()) {
                    result.plainText += text->Value();
                    continue;
                }
                auto* el = node->ToElement();
                if (!el) continue;

                std::string_view tag = el->Name();

                if (tag == "at") {
                    At at;
                    tryAttr(el, "id",   at.id);
                    tryAttr(el, "name", at.name);
                    tryAttr(el, "role", at.role);
                    tryAttr(el, "type", at.type);
                    result.ats.push_back(std::move(at));

                } else if (tag == "img") {
                    Image img;
                    if (auto v = el->Attribute("src")) img.src = v;
                    tryAttr(el, "title", img.title);
                    result.images.push_back(std::move(img));

                } else if (tag == "audio") {
                    Audio audio;
                    if (auto v = el->Attribute("src")) audio.src = v;
                    tryAttr(el, "title", audio.title);
                    result.audios.push_back(std::move(audio));

                } else if (tag == "video") {
                    Video video;
                    if (auto v = el->Attribute("src")) video.src = v;
                    tryAttr(el, "title", video.title);
                    result.videos.push_back(std::move(video));

                } else if (tag == "file") {
                    File file;
                    if (auto v = el->Attribute("src")) file.src = v;
                    tryAttr(el, "title", file.title);
                    result.files.push_back(std::move(file));

                } else if (tag == "quote") {
                    Quote quote;
                    tryAttr(el, "id", quote.id);
                    result.quotes.push_back(std::move(quote));
                }
                // 装饰元素(b/i/u/code等)只取文本内容，归入plainText
                else if (auto* t = el->GetText()) {
                    result.plainText += t;
                }
            }
            return result;
        }
        
    }
}
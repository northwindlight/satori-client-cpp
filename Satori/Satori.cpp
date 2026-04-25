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

        void to_json(nlohmann::json& j, const Channel& c)
        {
            j["id"] = c.id;
            j["type"] = c.type;
            if (c.name.has_value()) j["name"] = c.name.value();
            if (c.parent_id.has_value()) j["parent_id"] = c.parent_id.value();
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

        void to_json(nlohmann::json& j, const GuildRole& r)
        {
            j["id"] = r.id;
            if (r.name.has_value()) j["name"] = r.name.value();
        }

        void from_json(const nlohmann::json& j, GuildMember& m)
        {
            opt(j, "user",      m.user);
            opt(j, "nick",      m.nick);
            opt(j, "avatar",    m.avatar);
            opt(j, "joined_at", m.joined_at);
            opt(j, "roles",     m.roles);
        }

        void from_json(const nlohmann::json& j, Message& m)
        {
            j.at("id").get_to(m.id);
            opt(j, "content", m.content);
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
            opt(j, "status",  l.status);
            opt(j, "adapter", l.adapter);
            opt(j, "platform", l.platform);
            opt(j, "user",     l.user);
            opt(j, "features", l.features);
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

        void from_json(const nlohmann::json& j, Emoji& e)
        {
            j.at("id").get_to(e.id);
            opt(j, "name", e.name);
        }

        void from_json(const nlohmann::json& j, Friend& f)
        {
            opt(j, "user", f.user);
            opt(j, "nick", f.nick);
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
            opt(j, "emoji",    e.emoji);
            opt(j, "friend",   e.friend_);
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

        // ── Builder ─────────────────────────────────────────────

        Builder::Builder() {
            root = doc.NewElement("_");
            doc.InsertFirstChild(root);
        }

        Builder& Builder::at(const std::string& id, const std::string& name) {
            auto* el = doc.NewElement("at");
            el->SetAttribute("id", id.c_str());
            if (!name.empty()) el->SetAttribute("name", name.c_str());
            root->InsertEndChild(el);
            return *this;
        }

        Builder& Builder::atAll() {
            auto* el = doc.NewElement("at");
            el->SetAttribute("type", "all");
            root->InsertEndChild(el);
            return *this;
        }

        Builder& Builder::atHere() {
            auto* el = doc.NewElement("at");
            el->SetAttribute("type", "here");
            root->InsertEndChild(el);
            return *this;
        }

        Builder& Builder::sharp(const std::string& id, const std::string& name) {
            auto* el = doc.NewElement("sharp");
            el->SetAttribute("id", id.c_str());
            if (!name.empty()) el->SetAttribute("name", name.c_str());
            root->InsertEndChild(el);
            return *this;
        }

        Builder& Builder::text(const std::string& content) {
            root->InsertEndChild(doc.NewText(content.c_str()));
            return *this;
        }

        Builder& Builder::img(const std::string& src) {
            auto* el = doc.NewElement("img");
            el->SetAttribute("src", src.c_str());
            root->InsertEndChild(el);
            return *this;
        }

        Builder& Builder::audio(const std::string& src) {
            auto* el = doc.NewElement("audio");
            el->SetAttribute("src", src.c_str());
            root->InsertEndChild(el);
            return *this;
        }

        Builder& Builder::video(const std::string& src) {
            auto* el = doc.NewElement("video");
            el->SetAttribute("src", src.c_str());
            root->InsertEndChild(el);
            return *this;
        }

        Builder& Builder::file(const std::string& src, const std::string& title) {
            auto* el = doc.NewElement("file");
            el->SetAttribute("src", src.c_str());
            if (!title.empty()) el->SetAttribute("title", title.c_str());
            root->InsertEndChild(el);
            return *this;
        }

        Builder& Builder::br() {
            root->InsertEndChild(doc.NewElement("br"));
            return *this;
        }

        Builder& Builder::bold(const std::string& content) {
            auto* el = doc.NewElement("b");
            el->SetText(content.c_str());
            root->InsertEndChild(el);
            return *this;
        }

        Builder& Builder::italic(const std::string& content) {
            auto* el = doc.NewElement("i");
            el->SetText(content.c_str());
            root->InsertEndChild(el);
            return *this;
        }

        Builder& Builder::code(const std::string& content) {
            auto* el = doc.NewElement("code");
            el->SetText(content.c_str());
            root->InsertEndChild(el);
            return *this;
        }

        Builder& Builder::quote(const std::string& id) {
            auto* el = doc.NewElement("quote");
            el->SetAttribute("id", id.c_str());
            root->InsertEndChild(el);
            return *this;
        }

        std::string Builder::build() {
            std::string result;
            for (auto* node = root->FirstChild(); node; node = node->NextSibling()) {
                tinyxml2::XMLPrinter printer(nullptr, true);
                node->Accept(&printer);
                result += printer.CStr();
            }
            return result;
        }

    }
}
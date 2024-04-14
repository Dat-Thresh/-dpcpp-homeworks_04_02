#include <fstream>
#include <iostream>
#include <memory>

class Printable
{
public:
    virtual ~Printable() = default;

    explicit Printable(std::string data):data_(data){};
    virtual std::string print() const = 0;
    std::string data_;
};

class HTML_print:public Printable {

public:
    using Printable::Printable;
    std::string print() const override {
        return "<html>" + data_ + "<html/>";
    }
};

class Text_print :public Printable {
public:
    using Printable::Printable;
    std::string print() const override {
        return data_;
    }
};

class JSON_print : public Printable {
    using Printable::Printable;
    std::string print() const override {
        return "{ \"data\": \"" + data_ + "\"}";
    }
};

class Data
{
public:
    enum class Format
    {
        kText,
        kHTML,
        kJSON
    };
   
    explicit Data(std::unique_ptr<Data> next, std::string data) : next_(std::move(next)), data_(data) {};

    virtual std::unique_ptr<Printable> GetPrinter(Format format) {
        if (handleFormat(format)) {

                return std::move(printable_);

        }
        if(!next_) {
            throw std::exception("Format is not supported");
        }
        return next_->GetPrinter(format);
    }    

    virtual ~Data() = default;
private:  
    virtual bool handleFormat(Format& format) = 0;   
    std::unique_ptr<Data> next_;
protected:   
    std::unique_ptr<Printable> printable_;
    std::string data_;
    

};


class Text :public Data {
public:
    using Data::Data;


private:
    bool handleFormat(Format& format) override {
        if (format == Format::kText) {
            printable_ = std::make_unique<Text_print>(data_);
            return true;
        }
        return false;
    }
};

class HTML : public Data {
public:
    using Data::Data;
private:
    bool handleFormat(Format& format) override{
        if (format == Format::kHTML) {
            printable_ = std::make_unique<HTML_print>(data_);
            return true;
        }
        return false;
    }
};

class JSON : public Data {
public:
    using Data::Data;
private:
    bool handleFormat(Format& format) override {
        if (format == Format::kJSON) {
            printable_ = std::make_unique<JSON_print>(data_);
            return true;
        }
        return false;
    }

};


void saveTo(std::ofstream & file, std::unique_ptr<Data> data, Data::Format format) {
    auto printer_ptr = data->GetPrinter(format);
  
    file << printer_ptr->print();

}

void saveToAsHTML(std::ofstream& file, std::unique_ptr<Data> data) {
   saveTo(file, std::move(data), Data::Format::kHTML);
}

void saveToAsJSON(std::ofstream& file, std::unique_ptr<Data> data) {
   saveTo(file, std::move(data), Data::Format::kJSON);
}

void saveToAsText(std::ofstream& file, std::unique_ptr<Data> data) {
    saveTo(file, std::move(data), Data::Format::kText);
}
auto make_data_handler(std::string data) {
    std::unique_ptr<JSON> json_handle = std::make_unique<JSON>(nullptr, data);
    auto text_handle = std::make_unique<Text>(std::move(json_handle), data);
    auto html_handle = std::make_unique<HTML>(std::move(text_handle), data);
    return std::move(html_handle);
};

int main() {
    try {
        std::string data = "Test text";
       /* std::unique_ptr<JSON> json_handle = std::make_unique<JSON>(nullptr,data);
        auto text_handle = std::make_unique<Text>(std::move(json_handle), data);
        auto html_handle = std::make_unique<HTML>(std::move(text_handle), data);*/

        auto handler = make_data_handler(data);
        auto handler_1 = make_data_handler(data);
        auto handler_2 = make_data_handler(data);

        std::ofstream file;
        file.open("..\\test.txt", std::ios::app);

        //saveTo(file, std::move(html_handle), Data::Format::kJSON);

        //доделать логику передачи поинтера
        saveToAsHTML(file, std::move(handler));
        saveToAsJSON(file,std::move(handler_1));
        saveToAsText(file, std::move(handler_2));

        file.close();
 
    }
    catch (std::exception& er) {
        std::cout << er.what();
    }
   return 0;
}

//
// Created by vorta on 2/24/2021.
//

#ifndef SYSTEMICAI_COMMON_EXCEPTION_H
#define SYSTEMICAI_COMMON_EXCEPTION_H

namespace systemicai::common {
  struct exception : public std::exception {
    exception(const std::string&& msg) : _msg(msg) { ; }
    exception(const exception& e) : _msg(e._msg) { ; }
    exception(const exception&& e) : _msg(std::move(e._msg)) { ; }
    bool operator==(const exception& e) { return _msg == e._msg; }
    virtual ~exception() { ; }
    const char* what() const noexcept { return _msg.data(); }
    const std::string _msg;
  };

}

#endif // SYSTEMICAI_COMMON_EXCEPTION_H
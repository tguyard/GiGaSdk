/*
 * Locale.h
 *
 *  Created on: 30 sept. 2015
 *      Author: thomas
 */

#ifndef LOCALE_H_
#define LOCALE_H_

#include <boost/optional.hpp>
#include <cpprest/details/basic_types.h>
#include "../../rest/prepoc_manage.h"

struct Locale {
    utility::string_t contryCode  = U(U("FR"));
    utility::string_t countryName = {};
    utility::string_t currency    = {};
    boost::optional<utility::string_t> ip = boost::none;

    template <class Unserializer>
    void visit(const Unserializer& us){
        us.manageOpt(contryCode, utility::string_t(U("contryCode")), utility::string_t(U("FR")));
        us.manageOpt(countryName, utility::string_t(U("countryName")), utility::string_t(U("France")));
        us.manage(currency, utility::string_t(U("currency")));
        us.manage(ip, utility::string_t(U("ip")));
    }
};

#endif /* LOCALE_H_ */

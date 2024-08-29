#ifndef MODELFACTORY_HPP
#define MODELFACTORY_HPP

#include "interfaces/models/IModelDnnDetector.hpp"

#include <memory>
#include <string>
#include <map>

struct ModelFactory {
    static std::map<std::string, std::unique_ptr<IModelDnnDetector>(*)(const std::string&, const ODCaps, const void*)> factory;
};

#endif // MODELFACTORY_HPP
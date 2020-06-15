#pragma once
#include <string>
#include <memory> 
#include <unordered_map>
#include <mutex>
#include <typeinfo>

#include "Log.h"
#include "../lib/Json.h"
namespace Toy
{
class Cast
{
public:

private:

};

class ConfigVarBase
{
public:
	typedef std::shared_ptr<ConfigVarBase> Ptr;

	ConfigVarBase(const std::string & name, const std::string & description) 
		: m_name(name), m_description(description) {}
	virtual ~ConfigVarBase() {}

	const std::string & getName() const { return m_name; }
	const std::string & getDescription() const { return m_description; }
	virtual std::string toString() = 0;
	virtual bool fromString(const std::string & val) = 0;

protected:
    // ConfigVar Name
	std::string m_name;
    // Description of ConfigVar
	std::string m_description;
private:
}; // class ConfigVarBase

template<class T>
class ConfigVar : public ConfigVarBase
{
public:
	typedef std::shared_ptr<ConfigVar> Ptr;
	ConfigVar(const std::string & name, const T & val, const std::string & description = "")
		: ConfigVarBase(name, description), m_val(val) {}
	~ConfigVar() {}

	T getVal() const { return m_val; }
	void setVal(const T & val) { m_val = val; }
	virtual std::string toString() override
	{
		return std::string();
		//TODO
	}
	virtual bool fromString(const std::string & val) override
	{
		return false;
		//TODO
	}


private:
    // Value of ConfigVar
	T m_val;
}; // class ConfigVar

class Config
{
public:
	typedef std::unordered_map<std::string, ConfigVarBase::Ptr> ConfigVarMap;
	Config();
	~Config();

    /**
     * @brief Find the ConfigVar through name or create 
     * a new ConfigVar if it is not existed.
     * 
     * @tparam T 
     * @param name ConfigVar Name
     * @param val ConfigVar Value
     * @param description Description of ConfigVar
     * @return ConfigVar<T>::Ptr 
     */
	template<typename T>
	static typename ConfigVar<T>::Ptr lookup(const std::string & name,
		const T & val, const std::string & description = "")
	{
		auto tmp = lookup<T>(name);
		if (tmp != nullptr)
		{
			TOY_LOG_INFO << "Lookup name : " << name << " exists.";
			return tmp;
		}
		if(name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ._0123456789[]") 
			!= std::string::npos)
		{
			TOY_LOG_ERROR << "Look up name :" << name << " invaild.";
			throw std::invalid_argument(name);
		}
		tmp = std::make_shared<ConfigVar<T>>(name, val, description);
		std::unique_lock<std::mutex> lock(s_mutex);
		s_config_vars[name] = tmp;
		return tmp;

	}
    
	/**
     * @brief Find the ConfigVar through name, return nullptr if it's not existed.
     * 
     * @tparam T 
     * @param name 
     * @return ConfigVar<T>::Ptr 
     */
	template<typename T>
	static typename ConfigVar<T>::Ptr lookup(const std::string & name)
	{
		std::unique_lock<std::mutex> lock(s_mutex);
		auto it = s_config_vars.find(name);
		if (it != s_config_vars.end())
			return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
		else
			return nullptr;
	}
	static bool loadConfig(const std::string & file_name)
	{
		JsonVar jv;
		if(!Json::loadFile(file_name.c_str(), jv))
			return false;
		transformJV2CV(jv, "");
		return true;
	}
	// print all ConfigVar
	static void viewAll()
	{
		std::string type1(typeid(ConfigVar<double>).name());
		std::string type2(typeid(ConfigVar<std::string>).name());
		std::string type3(typeid(ConfigVar<bool>).name());
		for(const auto & cvp : s_config_vars)
		{
			std::string type_name(typeid(*cvp.second).name());

			if(type_name == type1)
				printCV<double>(cvp.second);
			else if(type_name == type2)
				printCV<std::string>(cvp.second);
			else if(type_name == type3)
				printCV<bool>(cvp.second);
			else
				std::cout << type_name << std::endl;
		}
	}
private:
	/**
	 * @brief transfrom JsonVar to ConfigVar, and store it into ConfigVarMap
	 * 
	 * @param jv JsonVar
	 * @param str Passing by the upper level
	 */
	static void transformJV2CV(const JsonVar & jv, std::string str);
	
	template<typename T>
	static void printCV(const ConfigVarBase::Ptr & cvp)
	{
		std::cout << "CV Name : "  << cvp->getName()
			<< ";\t Value : " << (std::dynamic_pointer_cast<ConfigVar<T>>(cvp))->getVal()
			<< ";\t Description : " << cvp->getDescription()
			<< std::endl;
	}
	
	static std::mutex s_mutex;          // protect ConfigVarMap
	static ConfigVarMap s_config_vars;

}; // class Config

} // namespace Toy
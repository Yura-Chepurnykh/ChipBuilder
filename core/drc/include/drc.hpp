#ifndef DRC_HPP
#define DRC_HPP

#include <memory>
#include <vector>
#include <list>
#include <string>
#include <stdexcept>
#include "parameter_object.hpp"

// we use chain of responsibility pattern 
// for drc 

class Rule;

class DRCDirector 
{
public:
    void add(std::shared_ptr<Rule>);
    void runAll();

private:
    std::list<std::weak_ptr<Rule>> m_rules;
};

class Rule 
{
public:
    Rule();
    virtual ~Rule() = default;
    void setNext(std::shared_ptr<Rule>);
    virtual void handle(ParameterObject&);

protected:
    std::shared_ptr<Rule> m_next;    
};

class MinimumWidth : public Rule 
{
public:
    MinimumWidth(int) noexcept; 
    void handle(ParameterObject&) override;

private:
    int m_minWidth;
};

class MinimumSpacing : public Rule 
{
public:
    MinimumSpacing(int) noexcept;
    void handle(ParameterObject&) override;

private:
    int m_minSpace;
};

class MinimumArea : public Rule 
{
public:
    MinimumArea(int) noexcept;
    void handle(ParameterObject&) override;

private:        
    int m_minArea;
};

class Enclosure : public Rule 
{
public:
    void handle(ParameterObject&) override;
};

class Overlap : public Rule 
{
public:
    void handle(ParameterObject&) override;
};

class Extension : public Rule 
{
public:
    void handle(ParameterObject&) override;
};

class Notch : public Rule 
{
public:
    void handle(ParameterObject&) override;
};

class Density : public Rule 
{
public:
    void handle(ParameterObject&) override;
};

class Well : public Rule
{
public:
    void handle(ParameterObject&) override;
};

class Implant : public Rule 
{ 
public:
    void handle(ParameterObject&) override;
};

#endif // DRC_HPP

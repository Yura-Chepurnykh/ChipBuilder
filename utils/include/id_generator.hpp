#ifndef ID_GENERATOR_HPP
#define ID_GENERATOR_HPP

class IdGenerator
{
public:
    static unsigned int generate()
    {
        auto currId = m_id;
        ++m_id;
        return currId;
    }

private:
    static constinit unsigned int m_id = 0;
};

#endif // ID_GENERATOR_HPP


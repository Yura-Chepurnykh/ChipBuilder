#ifndef ID_GENERATOR_HPP
#define ID_GENERATOR_HPP

class IdGenerator
{
public:
    static unsigned int generate()
    {
        auto id = m_id;
        ++m_id;
        return id;
    }

private:
    static inline unsigned int m_id = 0;
};

#endif // ID_GENERATOR_HPP

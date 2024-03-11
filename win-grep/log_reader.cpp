#include "log_reader.h"

#include <string.h>
#include <stdexcept>


size_t DEFAULT_MASK_SUBSTR_SIZE_LIMIT           = 255;


CLogReader::~CLogReader() {
    if (m_buffer)
        delete[] m_buffer;

    if (m_filter_action)
        delete m_filter_action;

    Close();
}


bool CLogReader::Open(char const *file_name) {
    if (m_file)
        return false; // ----->

    m_lpos   = 0;
    m_rpos   = 0;

    return !::fopen_s(&m_file, file_name, "rb"); // ----->
}


void CLogReader::Close() {
    if (m_file) {
        fclose(m_file);
        m_file      = nullptr;
        m_is_eof    = false;
    }
}


template<typename T>
void swap(T &value1, T &value2) {
    auto value = value1;
    value1 = value2;
    value2 = value;
}


bool CLogReader::SetFilter(const char *filter) {
    if (m_filter_action) {
        delete m_filter_action;
        m_filter_action = nullptr;
    }

    TFilterAction *action = new (std::nothrow) TFilterAction();

    if (action)
        m_filter_action = action;
    else
        return false; // ----->

    size_t size = strnlen_s(filter, DEFAULT_BUFFER_SIZE);
    size_t lpos = 0;
    while (lpos < size && filter[lpos] > ' ') {
        action->next = new (std::nothrow) TFilterAction();
        if (!action->next)
            break; // --->

        if (filter[lpos] == '?') {
            auto rpos = lpos;

            while (filter[rpos] == '?' && rpos < size)
                rpos++;

            if (rpos > DEFAULT_MASK_SUBSTR_SIZE_LIMIT)
                break; // --->

            action->next->operation  = TFilterAction::TOperation::SKIP;
            action->next->size       = rpos - lpos;

            action  = action->next;
            lpos    = rpos;

            continue; // <---
        }

        action->next->operation = TFilterAction::TOperation::COMPARE;

        // skip '*', op = find
        if (filter[lpos] == '*') {
            action->next->operation = TFilterAction::TOperation::FIND;
            while (filter[lpos] == '*' && lpos < size)
                lpos++;
        }

        auto rpos = lpos;

        // word position (lpos ... rpos) after '*' 
        while (
            rpos < size &&
            filter[rpos] != '*' &&
            filter[rpos] != '?') 
        {
            if (filter[rpos] == '\\' && rpos + 1 < size &&
               (filter[rpos + 1] == '\\' ||
                filter[rpos + 1] == '*'  ||
                filter[rpos + 1] == '?'))
            {
                rpos++;
            }

            if (filter[rpos] == '\n') {
                delete m_filter_action;
                m_filter_action = nullptr;
                return false; // ----->
            }

            rpos++;
        }

        // copy substr for finding after '*' or for comparing
        if (rpos > lpos) {
            size_t  size        = rpos - lpos;
            action->next->data  = new (std::nothrow) char[size + 1];

            if (action->next->data) {
                int i = 0;
                while (lpos < rpos && i < size) {
                    if (filter[lpos]     == '\\' && lpos + 1 < size &&
                       (filter[lpos + 1] == '\\' ||
                        filter[lpos + 1] == '*'  ||
                        filter[lpos + 1] == '?')) 
                    {
                        lpos++;
                    }
                    action->next->data[i] = filter[lpos];
                    lpos++;
                    i++;
                }
                action->next->data[i] = 0;

                //action->next->data = data;
                action->next->size = i;
            } else
                break; // --->
        }

        if (action->next)
            action = action->next;
        else {
            delete m_filter_action;
            m_filter_action = nullptr;
            return false; // ----->
        }

        lpos = rpos;
    }

    if (lpos < size) {
        delete m_filter_action;
        m_filter_action = nullptr;
        return false; // ----->
    } else
        m_filter_action = m_filter_action->next;

    // *?   -> ?*
    // *?*? -> ?(2)*
    {
        auto i = m_filter_action;
        while  (i->next) {
            if (i->operation        == TFilterAction::TOperation::FIND && !i->data &&
                i->next->operation  == TFilterAction::TOperation::SKIP) 
            {
                swap(i->operation, i->next->operation);
                swap(i->data     , i->next->data);
                swap(i->size     , i->next->size);

                auto skip = i;
                auto find = i->next;

                i = i->next;

                if (i->data)
                    continue; // --->

                while  (i->next) {
                    if (i->next->operation == TFilterAction::TOperation::FIND ||
                        i->next->operation == TFilterAction::TOperation::SKIP ||
                        i->next->operation == TFilterAction::TOperation::COMPARE)
                    {
                        if((i->next->operation == TFilterAction::TOperation::FIND ||
                            i->next->operation == TFilterAction::TOperation::COMPARE) &&
                            i->next->size > 0)
                        {
                            find->data = i->next->data;
                            find->size = i->next->size;
                            i->next->data = nullptr;
                        }

                        else

                        if (i->next->operation == TFilterAction::TOperation::SKIP)
                            skip->size += i->next->size;

                        auto next = i->next->next;
                        i->next->next = nullptr;
                        delete i->next;
                        i->next = next;

                        if (i->data)
                            break; // --->

                    } else
                        break; // --->
                }
            } else
                i = i->next;
        }
    }

    //{
    //    auto i = m_filter_action;
    //    while (i) {
    //        if (i->operation == TFilterAction::TOperation::COMPARE)
    //            std::cout << "compare: ";
    //        else
    //        if (i->operation == TFilterAction::TOperation::FIND)
    //            std::cout << "find   : ";
    //        else
    //        if (i->operation == TFilterAction::TOperation::SKIP)
    //            std::cout << "skip   : ";
    //        else
    //            std::cout << "unknown: ";
    //        std::cout << " '" << (i->data ? i->data : "null") << "' " << i->size << std::endl;
    //        i = i->next;
    //    }
    //}

    return m_filter_action; // ----->
}


bool CLogReader::GetNextLine(char *buffer, const int buffer_size) {
    if (m_file && m_filter_action && buffer_size > 0) {
        buffer[0] = 0;
        if (m_buffer) {
            if (m_buffer_size < buffer_size) {
                m_buffer_size = buffer_size;

                auto buffer = m_buffer;
                m_buffer    = new char[m_buffer_size];

                strcpy_s(m_buffer, m_rpos, buffer);
                delete[] buffer;
            }
        } else {
            m_buffer        = new char[buffer_size];
            m_buffer_size   = buffer_size;
            m_lpos          = 0;
            m_rpos          = 1;
            m_buffer[0]     = '\n';
            m_buffer[1]     = 0;
        }

        // left     position of filtering line
        size_t lpos = m_lpos;
        // right    position of filtering line
        size_t rpos = lpos + 1;
        // action   position of filtering line
        size_t apos = lpos + 1;

        // BUFFER = [m_lpos]...[lpos]='\n'...[apos]...[rpos]='\n'...[m_rpos]
        // apos = position between lpos and rpos compared  by action filter

        // read the file and parse the buffer until it finds a line by mask or receives eof
        do {
            // read the file until it finds a line like "\n...string...\n" with size less than buffer_size
            do {
                if (!m_is_eof && m_rpos < (m_buffer_size - 1)) {
                    auto size = fread(m_buffer + m_rpos, 1, m_buffer_size - m_rpos - 1, m_file);
                    if ( size > 0 ) {
                        rpos = m_rpos;
                        m_rpos += size;
                        m_is_eof = feof(m_file);

                        if (m_rpos < m_buffer_size)
                            m_buffer[m_rpos] = m_is_eof ? '\n' : 0;
                    } else
                        return false; // ----->
                }

                if (m_lpos >= m_rpos)
                    return false; // ----->

                while (lpos < m_rpos && 
                    m_buffer[lpos] == '\n' ||
                    m_buffer[lpos] == '\r'
                )
                    lpos++;

                rpos = lpos;
                lpos--;

                while (rpos < m_rpos && m_buffer[rpos] != '\n')
                    rpos++;

                // todo: refactiring, split to few conditions
                if(rpos > 1 && (
                   ((m_buffer[lpos] != '\n'  || lpos == rpos) &&
                     m_buffer[rpos] == '\n') ||
                    (m_buffer[lpos] == '\n' &&
                     m_buffer[rpos] != '\n' && rpos == m_rpos)))
                {
                    auto pos = m_buffer[rpos] == '\n' ? rpos : lpos;
                    auto size = m_rpos - pos;
                    if (memmove_s(m_buffer, m_buffer_size, &m_buffer[pos], size))
                        return false; // ----->
                    m_buffer[size] = 0;
                    m_lpos  = lpos = 0;
                    m_rpos  = size;
                    rpos    = lpos + 1;
                } 

                else

                if (m_buffer[rpos] != '\n') {
                    m_lpos = lpos = 0;
                    m_rpos = 0;
                }

            } while (m_rpos == 0 ||
               !(m_buffer[lpos] == '\n' &&
                 m_buffer[rpos] == '\n'));

            auto action = m_filter_action;

            apos = lpos + 1;

            if (apos >= m_rpos && m_is_eof)
                return false; // ----->

            while  (action) {
                if (apos + action->size > rpos)
                    break; // --->

                if (action->operation == TFilterAction::TOperation::COMPARE) {
                    if (strncmp(&m_buffer[apos], action->data, action->size))
                        break; // --->
                    else
                        apos += action->size;
                }

                else

                if (action->operation == TFilterAction::TOperation::FIND) {
                    if (action->data) {
                        m_buffer[rpos] = 0;
                        auto p = strstr(&m_buffer[apos], action->data);
                        m_buffer[rpos] = '\n';
                        if (p)
                            apos += action->size + p - &m_buffer[apos];
                        else
                            break; // --->
                    } else {
                        apos = rpos;
                    }
                }

                else

                if (action->operation == TFilterAction::TOperation::SKIP)
                    apos += action->size;

                action = action->next;
            }

            if (!action) {
                while (apos < rpos && 
                   (m_buffer[apos] == '\r' ||
                    m_buffer[apos] == '\n')) 
                {
                    apos++;
                }
            }

            if (action || apos < rpos) {
                lpos =   rpos;
                apos = ++rpos;

                if (rpos < (m_rpos - 1))
                    continue; // <---
                else {
                    rpos = lpos;
                    continue; // <---
                }
            } else {
                while (lpos < rpos &&
                   (m_buffer[lpos] == '\r' ||
                    m_buffer[lpos] == '\n')) 
                {
                    lpos++;
                }

                while (apos > lpos &&
                   (m_buffer[apos] == '\r' ||
                    m_buffer[apos] == '\n'))
                {
                    apos--;
                }
                apos++;

                auto size = apos - lpos;
                if (strncpy_s(buffer, buffer_size, &m_buffer[lpos], size)) {
                    return false; // ----->
                } else {
                    buffer[size] = 0;
                    m_lpos = rpos;
                    return true; // ----->
                }
            }
        } while (m_lpos < m_rpos || !m_is_eof);
    }

    return false; // ----->
}


CLogReader::TFilterAction::~TFilterAction() {
    if (next)
        delete   next;
    if (data)
        delete[] data;
}

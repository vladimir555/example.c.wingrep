#pragma once


#include <cstdio>


class CLogReader {
public:
    static size_t const DEFAULT_BUFFER_SIZE = 16384;//10;

    CLogReader() = default;
   ~CLogReader();

    // �������� �����, false - ������
    bool Open(const char *path);
    // �������� �����
    void Close();
    // ��������� ������� �����, false - ������
    bool SetFilter(const char *filter);
    // ������ ��������� ��������� ������,
    // buf      - �����, 
    // bufsize  - ������������ �����
    // false    - ����� ����� ��� ������
    bool GetNextLine(char *buf, const int bufsize);

private:
    struct TFilterAction {
        enum class TOperation {
            UNKNOWN,
            SKIP,
            FIND,
            COMPARE
        };

        TOperation      operation   = TOperation::UNKNOWN;
        char           *data        = nullptr;
        size_t          size        = 0;
        TFilterAction  *next        = nullptr;

       ~TFilterAction();
    };

    ::FILE         *m_file          = nullptr;
    TFilterAction  *m_filter_action = nullptr;
    char           *m_buffer        = nullptr;
    // buffer size
    size_t          m_buffer_size   = 0;
    // left  position of string in buffer
    size_t          m_lpos          = 0;
    // right position of string in buffer
    size_t          m_rpos          = 0;
    bool            m_is_eof        = false;
};

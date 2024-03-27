Упрощенный windows аналог grep

Параметры запуска win-grep.exe FILE.TXT "MASK"
где 
FILE.TXT  : любой текстовый файл с символами длиной 1 байт
MASK      : маска фильтрации строк
где
cимвол '*' - последовательность любых символов неограниченнои длины;
cимвол "?" - один любой символ;
cимвол "\" - экранирует "*", "?" или "\";

Бенчмарки:
powershell: 
  Measure-Command { Start-Process .\win-grep.exe -ArgumentList "FILE.TXT", "MASK" -NoNewWindow -Wait }
bash:
  time grep MASK FILE.TXT


Simplified Windows analogue of grep

Launch parameters win-grep.exe FILE.TXT “MASK”
Where
FILE.TXT: Any text file with 1 byte characters.
MASK : mask filters the string
Where
symbol '*' - a sequence of any characters of unlimited length;
symbol "?" - any one symbol;
symbol "\" - escapes "*", "?" or "\";

Benchmarks:
powershell:
  Measure-Command { Start-Process .\win-grep.exe -ArgumentList "FILE.TXT", "MASK" -NoNewWindow -Wait }
bash:
  time grep MASK FILE.TXT

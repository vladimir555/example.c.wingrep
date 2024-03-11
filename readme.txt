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

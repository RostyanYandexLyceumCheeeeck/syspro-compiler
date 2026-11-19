## Lexer
Находясь в корневой папке проекта:

1) соберите проект
```bash
cmake -S src/lexer -B build_lexer
cmake --build build_lexer
```

2) запустите лексер:
```bash
./build/bin/lexer path/to/file.spl
```

поддерживает все 5 грамматик(вроде как)
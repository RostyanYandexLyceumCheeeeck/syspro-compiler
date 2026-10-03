## Lexer
Находясь в корневой папке проекта:

1) соберите проект
```bash
cmake -S src/lexer -B build
cmake --build build
```

2) запустите лексер:
```bash
./build/lexer path/to/file.spl
```

поддерживает все 5 грамматик(вроде как)
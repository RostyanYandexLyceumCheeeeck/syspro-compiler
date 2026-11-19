## Parser
Находясь в корневой папке проекта:

1) соберите проект
```bash
cmake -S src/parser -B build_parser
cmake --build build_parser
```

2) запустите лексер:
```bash
./build/bin/parser path/to/file_tokens.json
```

поддерживает 3-ю грамматику(вроде как)
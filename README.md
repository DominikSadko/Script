# Example
```cpp
#include <iostream>

#include <Framework/Script/Engine.hpp>

int main()
{
       Script::Engine script;

       if (script.ExecuteRaw("print('Hello world!');"))
       {
               std::cout << "It works!" << std::endl;
       }

       return 0;
}
```

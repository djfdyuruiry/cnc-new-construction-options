---
description: C++ Coding Style
---

- Class names are in upper camel case: MyClass
- Method names are in upper snake case: My_Method
- Class members/fields are in upper camel case: MyMemberField
- Variables and parameters are in lower snake case: my_variable

- Class, Enum, Struct and Method braces start on a newline:

  ```c++
    My_Method()
    {
    }
  ```

- if, for, case, lambda and other control blocks have braces on the same line:

  ```c++
    while (true) {
      // do stuff
    }
  ```

- Class block order is public -> protected -> private

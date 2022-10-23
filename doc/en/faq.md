# Frequently Asked Questions
## 1. Ambiguous Call to `format` Function
This is caused by [argument-dependent lookup (ADL)](https://en.cppreference.com/w/cpp/language/adl) of C++. It usually occurs after `using namespace papilio` and using `format` to output types from the `std` namespace.  
Use `paplio::format` to explicitly specify the correct function to fix this.

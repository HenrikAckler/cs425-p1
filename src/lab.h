#ifndef LAB_H
#define LAB_H

/**
 * @brief Return a heap-allocated greeting for a name.
 *
 * The caller owns the returned string and must release it with free().
 * A NULL name is rejected and produces NULL.
 * @param name Name to include in the greeting.
 * @return Allocated greeting text, or NULL for invalid input or allocation failure.
 */
char* get_greeting(const char* restrict name);


#endif // LAB_H

/*  @file       linkedlist.h
*   @brief      Circular doubly linked list library
*   @details    This library implements a circular doubly linked list structure.
*   @author     Sefa Unal
*   @version    0.1
*   @date       13/12/2022
*
*   @todo       Implement thread safe functions
*
*   @since		v0.1 (13/12/2022) : Initial release
*/

#ifndef LINKEDLIST_H
#define LINKEDLIST_H

typedef struct List List;

typedef int (*CallBackFn)(void* data, void* userArg);

/** @brief Creates an empty linked list
 *
 * Initializes the structure and creates a buffer in heap.
 *
 * @pre Returns NULL pointer on error.
 *
 * @remarks Note that this function is not thread-safe!
 *
 * Typical usage example:
 * @code
 * List* list = list_open();
 * @endcode
 *
 * @return                            Pointer for the list handle.
 * @retval      NULL                : An error occured
 */
List* list_open();

/** @brief Appends a new element after its last element
 *
 * Typical usage example:
 * @code
 * List* list = list_open ();
 * typedef struct
 * {
 *     uint8_t a;
 *     uint8_t b[4];
 * } testObject;
 * testObject data[2];
 * List* list = list_append (list, data);
 * @endcode
 *
 * @param[in]   handle      : Handle for the structure
 * @param[in]   data        : Data to be added
 *
 * @return                    Pointer to the added element.
 * @retval      0           : Error
 */
int list_append(List* handle, void *data);

/** @brief Returns the data at specified index
 *
 * @param[in]   handle      : Handle for the structure
 * @param[in]   index       : Node index, accepts negative index
 *
 * @return                    Pointer to the data.
 * @retval      0           : Error
 */
void* list_get(List* handle, int index);

/** @brief Removes node at the specified index
 *
 * @param[in]   handle      : Handle for the structure
 * @param[in]   index       : Node index, accepts negative index
 *
 * @return                    Pointer to the removed data.
 * @retval      0           : Error
 */
void *list_remove(List* handle, int index);

/** @brief Loops through the nodes and calls the user function with the node data and given argument.
 * Removes node if user function returns a negative value.
 * Returns node data and exits the loop if user function returns a positive value.
 *
 * @param[in]   callbackFn  : Handle for the structure
 * @param[in]   callbackArg : Node index, accepts negative index
 *
 * @retval      0           : Error
 * @retval      1           : OK
 */
void* list_loop(List* handle, CallBackFn callbackFn, void *callbackArg);

/** @brief Returns size of the list
 *
 * @param[in]   handle      : Handle for the structure
 *
 * @retval      0           : Error
 */
int list_size(List *handle);

#endif // LINKEDLIST_H

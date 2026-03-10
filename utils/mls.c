#ifdef S_SPLINT_S
#define __BEGIN_DECLS
#endif

#define MLS_DEBUG_DISABLE
#include "mls.h"

#include <errno.h>
#include <limits.h>
#include <search.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static const char *Version =
    "Version:$Id: mls.c,v 1.1.1.1 2010-02-12 08:04:52 jens Exp $";

// HISTORY:
//
// 2007/11/23 nomux BUG: lst_new, m_new: LP->max not updated when array is
// resized
//
// 2007/12/18 nomux OPT: m_free, m_create: dont scan for free entries, just put
// them into a new list.
//
// 2007-12-25 nomux: m_free(): ignore h==0 instead of abort with error
//
// 2011-02-04 nomux: m_ins: added n==0 -> error, keine erlaubnis 0 elemente
// einzufÃ¼gen
//
// 2012-03-09 nomux: deb_err: removed "debi.me=0". es sollte verhindert werden
// das nach einem internen fehler noch ueberflÃ¼ssige melden erscheinen. dies
// funktionierte ganz und gar nicht. stattdessen wurde die
// fehler-rÃ¼ckverfolgung abgeschaltet.
//
// 2012-03-12 nomux: m_setlen verwendet jetzt lst_resize um neuen speicher zu
// allokieren

// 2014-02-16 lst_remove remove multiple items bug, m_init, _m_init do not
// terminate prog 2014-06-30 lst_resize, lst_create - fill allocated memory with
// 0 2014-07-09 m_utf8getchar - benutzt jetzt neue UTF8CHAR 2014-07-09 void
// m_qsort( int list, int(*compar)(const void *, const void *)) 2016-11-27
// mstr_to_long 2018-12-28 m_write, lst_write
//            lst_write bug: if m_len(m) and count=0 -> error
// 2019-03-13 BUG: buffer overflow in deb_xxx better use vsnprintf :-)
// 2021-05-11 m_next do nothing if list==0
// 2021-11-22 BUG: lst_resize - fill new memory with zero
// 2023-12-18 FEATURE: m_next : allow data==NULL
// 2024-03-20 m_free_list( (void) user_free( void*, void* ), void *user_data )
// 2024-06-09 BUG: s_index missing p++
// -----------------------------------------------------------------------------------------------------

struct lst_owner_st {
  int allocated;
  const char *fn, *fun;
  int ln;
};
typedef struct lst_owner_st lst_owner;

static int DEB = 0; // debug list

struct debug_info_st {
  char msg[500];
  const char *me, *fn, *fun;
  int ln, args, handle, index;
  const void *data;
};

static struct debug_info_st debi;
static int m_free_simple(int h); /* was: m_free */
static int AUTO_start = 0;
static int AUTO_ln = -1;
static const char * AUTO_fn = NULL;
static const char *AUTO_fun = NULL;


//
// Error Reporting
// Run-time TRACE
//

int trace_level = 0;
static char buf[4096];

/**
 * @brief Prints an error message and terminates the program.
 *
 * @param line The line number where the error occurred.
 * @param file The file name where the error occurred.
 * @param function The function name where the error occurred.
 * @param format The printf-style format string for the error message.
 * @param ... Additional arguments for the format string.
 */
void deb_err(int line, const char *file, const char *function,
             const char *format, ...) {

  va_list argptr;
  int err = errno;
  va_start(argptr, format);
  vsnprintf(buf, sizeof(buf), format, argptr);
  va_end(argptr);

  fprintf(stderr, "ERROR: %s Line: %d Function: %s\n%s\n", file, line, function,
          buf);
  // todo: error handler exception for errors in mls.c
  // debi.me=0; // do not inspect mls functions for debug info
  if (err)
    perror("");
  exit(1);
}

/**
 * @brief Prints a warning message.
 *
 * @param line The line number where the warning occurred.
 * @param file The file name where the warning occurred.
 * @param function The function name where the warning occurred.
 * @param format The printf-style format string for the warning message.
 * @param ... Additional arguments for the format string.
 */
void deb_warn(int line, const char *file, const char *function,
              const char *format, ...) {

  va_list argptr;

  va_start(argptr, format);
  vsnprintf(buf, sizeof(buf), format, argptr);
  va_end(argptr);

  fprintf(stderr, "WARNING: %s Line: %d Function: %s\n%s\n", file, line,
          function, buf);
}

/**
 * @brief Prints a trace message if the trace level is sufficient.
 *
 * @param l The trace level of the message.
 * @param line The line number where the trace occurred (unused).
 * @param file The file name where the trace occurred (unused).
 * @param function The function name where the trace occurred.
 * @param format The printf-style format string for the trace message.
 * @param ... Additional arguments for the format string.
 */
void deb_trace(int l, int line, const char *file, const char *function,
               const char *format, ...) {

  va_list argptr;
  (void)line;
  (void)file;

  va_start(argptr, format);
  vsnprintf(buf, sizeof(buf), format, argptr);
  va_end(argptr);

  fprintf(stderr, "[%d]%s: %s\n", l, function, buf);
}

//
// Error Checking:
//   lst
//     Out of Memory --> exit(1)
//     Wrong Args    --> exit(1)
//
//
//  mlsdb
//     saves caller
//     gathers informations on alloc/free
//
//
// ********************************************
//
// lst_XXX and
// m_XXX implementation
//
// ********************************************

static lst_t ML = 0; // stack allocated vars
static lst_t FR = 0; // stack freed vars

/**
 * @brief Prints the current size of the handle stack.
 *
 * @return Always returns 0.
 */
int print_stacksize() {
  printf("STACKSIZE: %d\n", ML->l);
  return 0;
}

/**
 * @brief Reallocates memory and clears newly allocated space.
 *
 * This function calls realloc and fills the newly allocated memory with zeros.
 * If realloc fails, it terminates the program with an error message.
 *
 * @param pBuffer Pointer to the memory block to be reallocated.
 * @param oldSize The current size of the memory block.
 * @param newSize The new desired size of the memory block.
 * @return A pointer to the reallocated memory block.
 */
void *reallocz(void *pBuffer, size_t oldSize, size_t newSize) {
  void *pNew = realloc(pBuffer, newSize);
  if (!pNew)
    ERR("could not realloc buffer");
  if (newSize > oldSize && pNew) {
    size_t diff = newSize - oldSize;
    void *pStart = ((char *)pNew) + oldSize;
    memset(pStart, 0, diff);
  }
  return pNew;
}

/**
 * @brief Returns a pointer to the element at the specified index in a list.
 *
 * @param l The list to access.
 * @param i The index of the element.
 * @return A pointer to the element at index i.
 * @note Terminates the program if the index is out of bounds.
 */
void *lst(lst_t l, int i) {
  if (i >= l->l || i < 0)
    ERR("Index(%d) out of Bounds", i);
  return &l->d[l->w * i];
}

/**
 * @brief Creates a new list.
 *
 * @param max Initial capacity of the list.
 * @param w Width of each element in bytes.
 * @return A pointer to the newly created list.
 * @note Terminates the program if memory allocation fails.
 */
lst_t lst_create(int max, int w) {
  lst_t l = (lst_t)calloc(1, (max * w) + sizeof(struct ls_st));
  if (!l)
    ERR("Out of Memory");
  l->max = max;
  l->l = 0;
  l->w = w;
  return l;
}

/**
 * @brief Allocates space for n items in a list.
 *
 * If necessary, the list's capacity is increased.
 *
 * @param LP Pointer to the list pointer (may be updated if the list is resized).
 * @param n Number of items to allocate space for.
 * @return The index of the first new item, or -1 on error.
 */
int lst_new(lst_t *LP, int n) {
  int max = (**LP).max;
  int len = (**LP).l;
  int space = max - len - n;

  // not enough space left
  if (space < 0) {
    if (space == -1) // resize by 1.5
      max = (max >> 1) + 1 + max;
    else
      max = len + n;
    int new_size = max * (**LP).w + sizeof(struct ls_st);
    int old_size = (**LP).max * (**LP).w + sizeof(struct ls_st);
    *LP = (lst_t)reallocz(*LP, old_size, new_size);
    (**LP).max = max;
  }
  (**LP).l += n;
  return len;
}

/**
 * @brief Resizes a list to a new length.
 *
 * @param LP Pointer to the list pointer (may be updated).
 * @param new_len The new length (number of elements).
 */
void lst_resize(lst_t *LP, int new_len) {
  int len = (**LP).l;
  if (new_len < 0)
    ERR("need new_size>=0. but new_size=%d", new_len);
  int new_size = new_len * (**LP).w + sizeof(struct ls_st);
  int old_size = (**LP).max * (**LP).w + sizeof(struct ls_st);
  if (new_size == old_size)
    return;
  *LP = (lst_t)reallocz(*LP, old_size, new_size);
  (**LP).max = new_len;
  if (new_len < len)
    (**LP).l = new_len;
}

/**
 * @brief Appends an item to a list.
 *
 * @param LP Pointer to the list pointer.
 * @param d Pointer to the data to be appended.
 * @return The index of the new item, or -1 on error.
 */
int lst_put(lst_t *LP, const void *d) {
  int n;
  if (!d)
    ERR("NULL-Ptr");
  n = lst_new(LP, 1);
  if (n >= 0)
    memcpy(lst(*LP, n), d, (*LP)->w);
  return n;
}

/**
 * @brief Iterates through a list.
 *
 * Gets the pointer to the element after *p and increments *p by one.
 *
 * @param l The list to iterate.
 * @param p Pointer to the current index. Initialize with -1.
 * @param data Pointer to a void pointer that will receive the element's address.
 * @return 1 if an element was returned, 0 if no more elements are available.
 */
int lst_next(lst_t l, int *p, void *data) {
  *p += 1;
  if (*p > l->l || *p < 0) {
    *p = (int)l->l;
    return 0;
  }
  if (*p == (int)l->l)
    return 0;
  if (data)
    *(void **)data = lst(l, *p);
  return 1;
}

/**
 * @brief Inserts n elements at position p in a list.
 *
 * @param lp Pointer to the list pointer.
 * @param p The insertion index.
 * @param n Number of elements to insert.
 * @return A pointer to the newly allocated space, or NULL if p is out of bounds.
 */
void *lst_ins(lst_t *lp, int p, int n) {
  int cnt;
  void *src, *dst;
  lst_t l = *lp;
  if ((uint)p > (uint)l->l)
    return NULL;
  if (l->l + n > l->max) { // Optimize resize if we resize by one
    int old_size = l->max * l->w + sizeof(struct ls_st);
    int new_max = l->l + n;
    if (new_max - l->max == 1)
      new_max <<= 1;
    l->max = new_max;
    int new_size = l->max * l->w + sizeof(struct ls_st);
    l = (lst_t)reallocz(l, old_size, new_size);
    *lp = l;
  }
  cnt = (l->l - p) * l->w;
  l->l += n;
  src = lst(l, p);
  if (cnt > 0) // Do not app
  {
    dst = lst(l, p + n);
    memmove(dst, src, cnt);
  }
  bzero(src, n * l->w);
  return src;
}

/**
 * @brief Deletes an item at position p in a list.
 *
 * @param l The list to modify.
 * @param p The index of the item to delete.
 */
void lst_del(lst_t l, int p) {
  void *dest, *src;
  size_t n;

  if (l->l > p + 1) {
    dest = lst(l, p);
    src = lst(l, p + 1);
    n = l->l - p - 1;
    if (n) {
      n *= l->w;
      memmove(dest, src, n);
    }
  }
  l->l--;
}

/**
 * @brief Removes n items from a list starting at position p.
 *
 * @param lp Pointer to the list pointer.
 * @param p The starting index.
 * @param n The number of items to remove.
 */
void lst_remove(lst_t *lp, int p, int n) {
  void *dest, *src;
  lst_t l = *lp;
  size_t len;

  if (n <= 0 || p < 0)
    return;
  if (p >= l->l)
    return;
  if (p + n >= l->l) {
    l->l = p;
    return;
  }

  dest = lst(l, p);
  src = lst(l, p + n);
  len = l->l - p - n;
  len *= l->w;
  memmove(dest, src, len);
  l->l -= n;
}

/**
 * @brief Returns a pointer to the element at the specified index without bounds checking against used length.
 *
 * @param l The list to access.
 * @param i The index.
 * @return A pointer to the element.
 */
void *lst_peek(lst_t l, int i) {
  if (i < 0 || i > l->max)
    ERR("Out of bounds");
  return &l->d[l->w * i];
}

/**
 * @brief Writes data into a list at position p.
 *
 * @param lp Pointer to the list pointer.
 * @param p The starting index.
 * @param data Pointer to the data to be copied.
 * @param n Number of elements to write.
 * @return 0 on success, -1 on error.
 */
int lst_write(lst_t *lp, int p, const void *data, int n) {
  void *mem;
  lst_t l = *lp;
  if (p < 0 || n < 0)
    return -1;
  if (n == 0)
    return 0;
  if (p + n > l->max) {
    l->max = p + n;
    l = (lst_t)realloc(l, l->max * l->w + sizeof(struct ls_st));
    if (!l)
      ERR("could not realloc");
    *lp = l; // write-back new ptr
  }
  if (p + n > l->l)
    l->l = p + n;
  mem = lst(l, p);
  memcpy(mem, data, n * l->w);
  return 0;
}

/**
 * @brief Reads data from a list into a buffer.
 *
 * @param l The list to read from.
 * @param p The starting index.
 * @param data Pointer to a buffer pointer. If *data is NULL, memory is allocated.
 * @param n Number of elements to read.
 * @return 0 on success.
 */
int lst_read(lst_t l, int p, void **data, int n) {
  if (p < 0 || n < 0 || data == NULL)
    ERR("Wrong arguments");
  if (*data == 0)
    *data = malloc(l->w * n);
  if (!*data)
    ERR("Out of Memory");
  memcpy(*data, lst(l, p), n * l->w);
  return 0;
}

// ********************************************
//
// array handling functions
//
//
// ********************************************

/**
 * @brief Internal function to retrieve a list structure from a handle.
 *
 * Checks if the list is initialized, allocated, and matches the UAF protection pattern.
 *
 * @param m The handle of the list.
 * @return A pointer to the list pointer (lst_t *).
 */
static inline lst_t *_get_list(int m) {
  if (ML == 0 || m < 1)
    ERR("Not initialized");
  lst_t *l = (lst_t *)lst(ML, m & 0xffffff);
  if (*l == NULL)
    ERR("List %d not allocated", m);
  if ((*l)->uaf_protection != (m >> 24)) {
    ERR("uaf protection pattern does not match, expected:%d, got:%d",
        (*l)->uaf_protection, (m >> 24));
  }

  return l;
}

/**
 * @brief Returns a pointer to the element at the specified index in the list handle.
 *
 * @param m The list handle.
 * @param i The index of the element.
 * @return A pointer to the element.
 */
void *mls(int m, int i) {
  lst_t *lp = _get_list(m);
  return lst(*lp, i);
}

/**
 * @brief Allocates n new elements in the list and returns the index of the first one.
 *
 * @param m The list handle.
 * @param n The number of elements to add.
 * @return The index of the first new element.
 */
int m_new(int m, int n) {
  lst_t *lp = _get_list(m);
  return lst_new(lp, n);
}

/**
 * @brief Adds a single element to the list and returns its address.
 *
 * @param m The list handle.
 * @return A pointer to the new element.
 */
void *m_add(int m) { return mls(m, m_new(m, 1)); }

/**
 * @brief Resizes the list to a new capacity.
 *
 * @param m The list handle.
 * @param new_size The new capacity in number of elements.
 */
void m_resize(int m, int new_size) {
  lst_t *lp = _get_list(m);
  return lst_resize(lp, new_size);
}

/**
 * @brief Removes n items from the list starting at position p.
 *
 * @param m The list handle.
 * @param p The starting index.
 * @param n The number of items to remove.
 */
void m_remove(int m, int p, int n) {
  lst_t *lp = _get_list(m);
  return lst_remove(lp, p, n);
}

/**
 * @brief Iterates through the list.
 *
 * @param m The list handle.
 * @param p Pointer to the current index. Initialize with -1.
 * @param d Pointer to a void pointer that will receive the element's address.
 * @return 1 if an element was returned, 0 otherwise.
 */
int m_next(int m, int *p, void *d) {
  if (!m || !m_len(m))
    return 0;
  lst_t *lp = _get_list(m);
  if (!d)
    ERR("Data address d is ZERO");
  return lst_next(*lp, p, d);
}

// static int deep_protect = 0;
static int MF = 0;
static void free_wrap(int m);
static void free_strings_wrap(int m);
static void free_list_wrap(int m);

/**
 * @brief Initializes the MLS memory management system.
 *
 * @return 0 on success, 1 if already initialized.
 */
int m_init() {
  static lst_t zero = 0;
  if (ML)
    return 1; // schon initialisiert
  ML = lst_create(100, sizeof(lst_t));
  lst_put(&ML, &zero);
  FR = lst_create(100, sizeof(int));
  // -- m_init ready -- 
  MF = m_create( 10, sizeof(void*) );
  void *p;
  p =  free_wrap; m_put( MF, &p );
  p =  free_strings_wrap; m_put( MF, &p );
  p =  free_list_wrap; m_put( MF, &p );
  return 0;
}

/**
 * @brief Destroys the MLS memory management system and frees all lists.
 */
void m_destruct() {
  int p;
  lst_t *d;
  if (!ML)
    ERR("Not Init.");
  m_free_simple(MF);
  // -- m_destruct start -- 
  for (p = -1; lst_next(ML, &p, &d);)
    if (*d) {
      free(*d);
      TRACE(1, "m_free %d\n", p);
    }
  free(ML);
  ML = 0;
  free(FR);
  FR = 0;
}

static int UAF_PROTECTION = 0;

/**
 * @brief Creates a new list and returns its handle.
 *
 * @param max Initial capacity.
 * @param w Width of each element in bytes.
 * @return The list handle.
 */
int m_create(int max, int w) {
  int i;
  lst_t lp;
  if (!ML || max < 0 || w <= 0)
    ERR("Wrong args");
  lp = lst_create(max, w);
  lp->uaf_protection = UAF_PROTECTION;

  // falls FR->l > 0 nehme freie plätze aus FR
  if (FR->l > 0) { /* re-use old handles, common case */
    i = *(int *)lst(FR, FR->l - 1);
    FR->l--;
    *(lst_t *)lst(ML, i) = lp;
  } else { /* create new handle */
    i = lst_put(&ML, &lp);
    if (i >= 0xffffff)
      ERR("too many arrays allocated");
  }

  i = (UAF_PROTECTION << 24) | i;
  return i;
}

/**
 * @brief Frees the memory associated with a list handle without calling custom free handlers.
 *
 * @param h The list handle.
 * @return 0 on success.
 */
static int m_free_simple(int h) {
  if (!ML || h < 0)
    ERR("Wrongs Args ML=%p h=%d", ML, h);
  if (!h)
    return 0;
  lst_t *l = _get_list(h);
  /* uaf protection */
  {
    h &= 0xffffff; /* do not store uaf protection */
                   /* pattern as part of handle to be reused */
    UAF_PROTECTION = (UAF_PROTECTION + 1) & 0x7f;
  }

  free(*l);
  *l = 0;
  TRACE(1, "Free List %d", h);
  lst_put(&FR, &h);
  return 0;
}

/**
 * @brief Appends data to a list.
 *
 * @param m The list handle.
 * @param data Pointer to the data to append.
 * @return The index of the new item, or -1 on error.
 */
int m_put(int m, const void *data) {
  lst_t *lp = _get_list(m);
  return lst_put(lp, data);
}

/**
 * @brief Returns the number of used elements in a list.
 *
 * @param m The list handle.
 * @return The number of elements.
 */
int m_len_simple(int m) {
  lst_t *lp = _get_list(m);
  return (**lp).l;
}

/**
 * @brief Returns the number of used elements in a list.
 *
 * @param m The list handle.
 * @return The number of elements.
 */
int m_len(int m) {
  return m_len_simple(m);
}

/**
 * @brief Returns a pointer to the first element of the list buffer.
 *
 * @param m The list handle.
 * @return A pointer to the buffer.
 */
void *m_buf(int m) { return m_peek(m, 0); }

/**
 * @brief Returns the current capacity of the list.
 *
 * @param m The list handle.
 * @return The capacity in number of elements.
 */
int m_bufsize(int m) {
  lst_t *lp = _get_list(m);
  return (**lp).max;
}

/**
 * @brief Inserts n elements at position p in a list.
 *
 * @param m The list handle.
 * @param p The insertion index.
 * @param n Number of elements to insert.
 * @return The position p.
 */
int m_ins(int m, int p, int n) {
  lst_t *lp;
  if (p < 0 || n <= 0)
    ERR("Wrong Args Start:%d Count:%d", p, n);
  lp = _get_list(m);
  lst_ins(lp, p, n);
  return p;
}

/**
 * @brief Removes and returns the last element from the list.
 *
 * @param m The list handle.
 * @return A pointer to the popped element, or NULL if empty.
 */
void *m_pop(int m) {
  lst_t *lp;
  lp = _get_list(m);
  if ((**lp).l < 1)
    return NULL;
  (**lp).l--;
  return (*lp)->d + ((*lp)->w * (*lp)->l);
}

/**
 * @brief Deletes the element at position p.
 *
 * @param m The list handle.
 * @param p The index of the element to delete.
 */
void m_del(int m, int p) {
  lst_t *lp;
  if (p < 0)
    return;
  lp = _get_list(m);
  lst_del(*lp, p);
}

/**
 * @brief Clears the list (sets length to zero) without freeing memory.
 *
 * @param m The list handle.
 */
void m_clear_simple(int m) {
  lst_t *lp;
  lp = _get_list(m);
  (**lp).l = 0;
}

/**
 * @brief Clears the list.
 *
 * @param m The list handle.
 */
void m_clear(int m) {
  m_clear_simple(m);
}

/**
 * @brief Sets the used length of the list.
 *
 * If the new length is greater than the capacity, the list is resized.
 *
 * @param m The list handle.
 * @param len The new length.
 * @return 0 on success.
 */
int m_setlen(int m, int len) {
  lst_t *lp;
  if (len < 0)
    ERR("Wrong Arg len=%d", len);
  lp = _get_list(m);
  if (len > (**lp).max)
    lst_resize(lp, len);
  (**lp).l = len;
  return 0;
}

/**
 * @brief Returns a pointer to the element at index i without bounds checking against used length.
 *
 * @param m The list handle.
 * @param i The index.
 * @return A pointer to the element.
 */
void *m_peek(int m, int i) {
  lst_t *lp;
  lp = _get_list(m);
  return lst_peek(*lp, i);
}

/**
 * @brief Writes n elements from data into the list at position p.
 *
 * @param m The list handle.
 * @param p The starting index.
 * @param data Pointer to the data to write.
 * @param n Number of elements to write.
 * @return The list handle.
 */
int m_write(int m, int p, const void *data, int n) {
  lst_t *lp;
  if (n <= 0)
    return m;
  lp = _get_list(m);
  lst_write(lp, p, data, n);
  return m;
}

/**
 * @brief Reads n elements from the list starting at p into the buffer data.
 *
 * @param m The list handle.
 * @param p The starting index.
 * @param data Pointer to a buffer pointer.
 * @param n Number of elements to read.
 * @return 0 on success.
 */
int m_read(int m, int p, void **data, int n) {
  lst_t *lp;
  lp = _get_list(m);
  return lst_read(*lp, p, data, n);
}

/**
 * @brief Returns the width of each element in the list.
 *
 * @param m The list handle.
 * @return The element width in bytes.
 */
int m_width(int m) {
  lst_t *lp;
  lp = _get_list(m);
  return (**lp).w;
}

/**
 * @brief Frees the memory associated with a list handle, calling a custom free handler if registered.
 *
 * @param m The list handle.
 * @return 0 on success.
 */
int m_free(int m)
{	
	if( m < 1 ) return 0;
	
	lst_t *lp = _get_list(m);
	uint8_t h = (*lp)->free_hdl;
	if (h == 255) return 0; /* this prevents recursion, if a list contains itself */
	if (h == 0 ) { /* the simple case first */
	  m_free_simple(m);
	  return 0;
	}

	if( h >= m_len(MF) ) {
		ERR("FREE Hander %d undefined", h );
	}
	(*lp)->free_hdl = 255; // mark this list as 'freeing in progress'
	void (**fn)(int m);
	fn = mls(MF,h);
	if(!*fn) { ERR("FREE Hander %d is NULL", h ); }
	(*fn)(m);
	/* clean this list, use a non on-debug-override function
	   because we could be called from a debug function */
	m_free_simple(m);

	return 0;
}

/**
 * @brief Registers a custom free function.
 *
 * @param n Unused.
 * @param free_fn The free function to register.
 * @return The handle of the registered free function.
 */
int m_reg_freefn( int n, void (*free_fn) (int m) )
{
	void (**fn)(int m);	
	int p;
	for(p=-1; m_next(MF, &p, &fn); ) {
		if( *fn == free_fn ) return p;
	}
	return m_put( MF, &free_fn);
}

/**
 * @brief Allocates a new list with a specific free handler.
 *
 * @param max Initial capacity.
 * @param w Element width.
 * @param free_hdl Handle of the free function.
 * @return The list handle.
 */
int m_alloc( int max, int w, uint8_t free_hdl )
{
	if( MF && free_hdl >= m_len_simple(MF) ) {
		ERR("FREE Hander %d undefined", free_hdl );
	}
	int h = m_create( max, w );
	lst_t *lp = _get_list(h);
	(*lp)->free_hdl = free_hdl;
	return h;
}

/**
 * @brief Returns the free handler index of a list.
 *
 * @param h The list handle.
 * @return The free handler index.
 */
int m_free_hdl( int h )
{
	lst_t *lp = _get_list(h);
	return (*lp)->free_hdl; 
}

/**
 * @brief Checks if a list handle has been freed.
 *
 * @param h The list handle.
 * @return 1 if freed, 0 otherwise.
 */
int m_is_freed(int h)
{
	if (!FR) return 0;
	h &= 0xffffff;
	for (int i = 0; i < FR->l; i++) {
		if (*(int*)lst_peek(FR, i) == h) return 1;
	}
	return 0;
}


// ********************************************
//
//  Debug-Function Implementation
//
// ********************************************
#define CASSERT(a, l, f, n) ASERR(a, "Caller %s() in %s:%d", (n), (f), (l))

/**
 * @brief Sets up the debug information for the current caller.
 *
 * @param me Name of the MLS function being called.
 * @param ln Line number in the caller's source file.
 * @param fn File name of the caller's source file.
 * @param fun Name of the calling function.
 * @param args Flags indicating what to check (bit 0: handle, bit 1: index, bit 2: data).
 * @param handle The list handle being operated on.
 * @param index The element index being operated on.
 * @param data Pointer to the data being operated on.
 */
static void _mlsdb_caller(const char *me, int ln, const char *fn,
                          const char *fun, int args, int handle, int index,
                          const void *data) {
  debi.me = me;
  debi.ln = ln;
  debi.fn = fn;
  debi.fun = fun;
  debi.args = args;
  debi.handle = handle;
  debi.index = index;
  debi.data = data;
}

/**
 * @brief Internal function to print an error message to stderr.
 *
 * @param format printf-style format string.
 * @param ... Arguments for the format string.
 */
static void perr(char *format, ...) {
  va_list argptr;
  va_start(argptr, format);
  vfprintf(stderr, format, argptr);
  fputc('\n', stderr);
  va_end(argptr);
}

/**
 * @brief Checks if a list handle is valid during debugging.
 *
 * @return 0 if valid, -1 if an error is detected.
 */
static int _mlsdb_check_handle() {
  lst_t *lp;
  lst_owner *o;
  int orig = debi.handle;
  int h = orig & 0xffffff;
  perr("Checking Handle %d, uaf protection: %d", h, orig >> 24);

  if (h < 1 || h > m_len(DEB)) {
    perr("Handle out of range (0 < %d < %d)", h, m_len(DEB) + 1);
    return -1;
  }

  lp = (lst_t *)lst(ML, h);
  if (*lp == NULL) {
    perr("List base address for handle %d is not allocated", h);
    return -1;
  }

  if ((*lp)->uaf_protection != (orig >> 24)) {
    perr("uaf protection pattern does not match, expected:%d, got:%d",
         (*lp)->uaf_protection, (orig >> 24));
    return -1;
  }

  o = (lst_owner *)mls(DEB, h - 1);

  if (!o || o->allocated != 42) {
    perr("Array was not allocated");
    return -1;
  }

  if (o->ln < 0) {
    perr("Array was previously removed by %s() in %s:%d", o->fun, o->fn, o->ln);
    return -1;
  }

  perr("Array was created by %s in %s:%d", o->fun, o->fn, o->ln);

  perr("Base Address Structure: %p\n"
       "Base Address Data (d):%p\nElem.Width (w):%d\n"
       "Buffer size(max):%d\nUsed Size(l):%d",
       *lp, (*lp)->d, (*lp)->w, (*lp)->max, (*lp)->l);

  return 0;
}

/**
 * @brief Checks if a list index is valid during debugging.
 *
 * @return 0 if valid, -1 if an error is detected.
 */
static int _mlsdb_check_index() {
  int i = debi.index, h = debi.handle;
  if (i < -1) {
    perr("Array Index %d should be >=-1\n", i);
    return -1;
  }

  if (i >= m_len(h)) {
    perr("Array Index out of bounds i=%d should be lower than m_len(%d)=%d.\n",
         i, h, m_len(h));
    return -1;
  }

  return 0;
}

/**
 * @brief Post-mortem analysis function called upon exit if an error occurred.
 */
void exit_error() {
  if (!debi.me)
    return;

  perr("\n"
       "POST MORTEM ANALYSER STARTED\n"
       "****************************\n"
       "ERROR in funtion: '%s'. Called by '%s:%d' in '%s'",
       debi.me, debi.fun, debi.ln, debi.fn);

  if (!ML) {
    perr("m_init not called");
    return;
  }

  if (debi.args & 1)
    if (_mlsdb_check_handle())
      return;

  if (debi.args & 2)
    if (_mlsdb_check_index())
      return;

  if (debi.args & 4)
    if (debi.data == NULL) {
      perr("No Ptr to Data given.");
      return;
    }
}

/**
 * @brief Initializes MLS with debug tracking enabled.
 *
 * @return 0 on success, 1 if already initialized.
 */
int _m_init() {
  /*  if( ML || DEB ) { ERR("mls/debug already initialized"); } */
  if (DEB)
    return 1;

  m_init();
  DEB = m_create(100, sizeof(lst_owner));
  atexit(exit_error);
  return 0;
}

/**
 * @brief Destroys MLS and checks for memory leaks (lists not freed).
 */
void _m_destruct() {
  // check for allocated lists
  lst_owner *o;
  int i;

  for (i = -1; m_next(DEB, &i, &o);) {
    if (o->allocated == 42 && o->ln > 0) {
      WARN("List %d still allocated. "
           "Source: %s() in %s:%d",
           i + 1, o->fun, o->fn, o->ln);
    }
  }
  m_free_simple(DEB);
  m_destruct();
  debi.me = NULL;
}

/**
 * @brief Debug version of m_create, tracking caller information.
 *
 * @param ln Line number.
 * @param fn File name.
 * @param fun Function name.
 * @param n Initial capacity.
 * @param w Element width.
 * @return List handle.
 */
int _m_create(int ln, const char *fn, const char *fun, int n, int w) {
  lst_owner *lo;
  int len, m, m_uaf;
  _mlsdb_caller(__FUNCTION__, ln, fn, fun, 0, 0, 0, 0);
  m_uaf = m_create(n, w);

  m = m_uaf & 0xffffff; /* uaf protection */
  len = m_len(DEB);
  if (m > len)
    m_new(DEB, m - len);
  lo = (lst_owner *)mls(DEB, m - 1);

  lo->ln = ln;
  lo->fn = fn;
  lo->fun = fun;
  lo->allocated = 42;
  TRACE(1, "NEW LIST %d allocated by %s:%d in %s", m, fun, ln, fn);
  return m_uaf;
}

/**
 * @brief Debug version of m_alloc, tracking caller information.
 *
 * @param ln Line number.
 * @param fn File name.
 * @param fun Function name.
 * @param n Initial capacity.
 * @param w Element width.
 * @param free_hdl Free handler index.
 * @return List handle.
 */
int _m_alloc(int ln, const char *fn, const char *fun, int n, int w, uint8_t free_hdl) {
	int h = _m_create(ln,fn,fun,n,w);
	lst_t *lp = _get_list(h);
	(*lp)->free_hdl = free_hdl;
	return h;
}

/**
 * @brief Debug version of m_free, tracking caller information.
 *
 * @param ln Line number.
 * @param fn File name.
 * @param fun Function name.
 * @param m List handle.
 * @return 0 on success.
 */
int _m_free(int ln, const char *fn, const char *fun, int m) {
  if (!m)
    return 0;

  if (m_is_freed(m)) {
    WARN("Attempt to free already freed list %d. Called by %s() in %s:%d", m, fun, fn, ln);
    return 0;
  }

  if(! AUTO_start++ ) {  
	  AUTO_ln = ln;
	  AUTO_fn = fn;
	  AUTO_fun = fun;
  }
  _mlsdb_caller(__FUNCTION__, AUTO_ln, AUTO_fn, AUTO_fun, 1, m, 0, 0);
  m_free(m);

  m &= 0xffffff; /* uaf protection */
  lst_owner *o = (lst_owner *)mls(DEB, m - 1);

  o->ln = -ln;
  o->fun = fun;
  o->fn = fn;
  TRACE(1, "Free List %d", m);
  AUTO_start--;
  return 0;
}

/**
 * @brief Debug version of m_buf, tracking caller information.
 *
 * @param ln Line number.
 * @param fn File name.
 * @param fun Function name.
 * @param m List handle.
 * @return Pointer to buffer.
 */
void *_m_buf(int ln, const char *fn, const char *fun, int m) {
  if (!m)
    return 0;
  _mlsdb_caller(__FUNCTION__, ln, fn, fun, 3, m, 0, 0);
  return m_buf(m);
}

/**
 * @brief Debug version of mls, tracking caller information.
 *
 * @param ln Line number.
 * @param fn File name.
 * @param fun Function name.
 * @param h List handle.
 * @param i Index.
 * @return Pointer to element.
 */
void *_mls(int ln, const char *fn, const char *fun, int h, int i) {
  _mlsdb_caller(__FUNCTION__, ln, fn, fun, 3, h, i, 0);
  return mls(h, i);
}

/**
 * @brief Debug version of m_next, tracking caller information.
 *
 * @param ln Line number.
 * @param fn File name.
 * @param fun Function name.
 * @param h List handle.
 * @param i Pointer to index.
 * @param d Pointer to data pointer.
 * @return 1 if found, 0 otherwise.
 */
int _m_next(int ln, const char *fn, const char *fun, int h, int *i, void *d) {
  _mlsdb_caller(__FUNCTION__, ln, fn, fun, 7, h, i ? *i : -1, d);
  return m_next(h, i, d);
}

/**
 * @brief Debug version of m_put, tracking caller information.
 *
 * @param ln Line number.
 * @param fn File name.
 * @param fun Function name.
 * @param h List handle.
 * @param d Pointer to data to append.
 * @return Index of new item.
 */
int _m_put(int ln, const char *fn, const char *fun, int h, const void *d) {
  _mlsdb_caller(__FUNCTION__, ln, fn, fun, 5, h, 0, d);
  return m_put(h, d);
}

/**
 * @brief Debug version of m_clear, tracking caller information.
 *
 * @param ln Line number.
 * @param fn File name.
 * @param fun Function name.
 * @param h List handle.
 */
void _m_clear(int ln, const char *fn, const char *fun, int h) {
  _mlsdb_caller(__FUNCTION__, ln, fn, fun, 1, h, 0, 0);
  m_clear(h);
}



/*
   -------------------------------------------------------------------------

   UTILITY

 -------------------------------------------------------------------------
*/

#undef MLS_DEBUG_DISABLE
#include "mls.h"

/**
 * @brief Simple wrapper for m_free_simple to avoid recursion.
 *
 * @param m List handle.
 */
static void free_wrap(int m)
{
	//if  MLS_DEBUG is enabled, we may be called by   _m_free() -> m_free() -> free_wrap() 
	//we need m_free_simple to avoid a loop
	//but wait: if we are called by free_list_wrap() and we are in debug mode we need to call
	// _m_free for the list to clear debug-list information
	m_free_simple(m);


#if 0
	_mlsdb_caller(__FUNCTION__,AUTO_ln, AUTO_fn,AUTO_fun, 1, m, 0, 0);
	m &= 0xffffff; /* uaf protection */
	lst_owner *o = (lst_owner *)mls(DEB, m - 1);
	o->ln = -ln;
	o->fun = fun;
	o->fn = fn;
	TRACE(1, "Free List %d", m);
#endif	
}

/**
 * @brief Wrapper to free all strings within a list.
 *
 * @param list List handle.
 */
static void free_strings_wrap(int list)
{
  int index;
  char **strp;
  TRACE(1, "Free List %d", list & 0xffffff ) ;
  if (list < 1)
    return;

  lst_t *lp = _get_list(list);
  for(index=-1; lst_next(*lp, &index, &strp); ) {
    if (*strp) {
      free(*strp);
      *strp = NULL;
    }
  }
}

/**
 * @brief Wrapper to free all lists contained within a list.
 *
 * @param m List handle.
 */
static void free_list_wrap(int m)
{
	TRACE(1, "Free List %d", m & 0xffffff );
	int p,*d;	
	m_foreach(m,p,d) m_free(*d);
	// list m is marked with free_hdl=255 before this function was
	// called, so we will not get a recursion
	// if this list 'm' contains itself       
}

/**
 * @brief Prints the MLS library version.
 */
void m_print_version() {
  puts("MLS - Secure, Easy, Low-Overhead Array-Memory-Mangement");
  puts(Version);
}

/**
 * @brief Fills the entire list buffer with zeros.
 *
 * @param m List handle.
 */
void m_bzero(int m) {
  lst_t lp = *_get_list(m);
  bzero(lp->d, lp->max * lp->w);
}

// ********************************************
//
// Helper functions, keep it simple
//
// ********************************************

/**
 * @brief Appends a character to a list.
 *
 * @param m List handle.
 * @param c Character to append.
 * @return The appended character.
 */
int m_putc(int m, char c)
{
	return *(char*)m_add(m)=c;
}

/**
 * @brief Appends an integer to a list.
 *
 * @param m List handle.
 * @param c Integer to append.
 * @return The appended integer.
 */
int m_puti(int m, int c)
{
	return *(int*)m_add(m)=c;
}

/**
 * @brief Frees all strings in a string list and optionally the list itself.
 *
 * @param list The string list handle.
 * @param CLEAR_ONLY If non-zero, only the strings are freed and the list is cleared.
 */
void m_free_strings(int list, int CLEAR_ONLY) {
  int index;
  char **strp;
  if (list < 1)
    return;
  for(index=-1; m_next(list, &index, &strp); ) {
    if (*strp)
      free(*strp);
    *strp = NULL;
  }
  if (CLEAR_ONLY)
    m_clear(list); // reset array size to zero
  else {
	  lst_t *lp = _get_list(list);
	  (*lp)->free_hdl = 0;
	  m_free(list); // free simple array and if ncc. debug info
  }
  
}

/**
 * @brief Splits the string `s` at each occurrence of the character `c` and
 * copies the handles to the resulting substrings into the array list `m`.
 *
 * - An empty string results in an entry with a string of length zero.
 * - A string containing only the separator results in a string list with two
 * entries, both empty.
 * - If `m` is set to 0, a new string list is created. Otherwise, the existing
 * list is cleared and used.
 * - If `remove_wspace` is non-zero, leading and trailing whitespace characters
 * in the resulting substrings are removed.
 *
 * @param m The string list to which the substrings are copied. If set to 0, a
 * new list is created.
 * @param s The input string to be split.
 * @param c The character used as the delimiter for splitting the string.
 * @param remove_wspace Flag to indicate whether leading and trailing whitespace
 * should be removed from the substrings.
 * @return The generated string list handle.
 */
int s_split(int m, const char *s, int c, int remove_wspace) {
  int p = 0, start = 0, end;
  char *szTemp;

  if (m)
	  m_free_strings(m, 1);
  else
	  m = m_alloc(10, sizeof(char *), MFREE_STR );

  for (;;) {

    // leading white-space
    if( remove_wspace ) while (isspace(s[p]) && s[p] != c)
      p++;
    start = p;

    // delimeter
    while (s[p] && s[p] != c)
      p++;

    //  trailing whitespace before delimeter, zero - length: end < start
    if (remove_wspace) {
        end = p;
        while (end > start && isspace(s[--end]))
            ;
        if (end >= start && !isspace(s[end])) {
            szTemp = strndup(s + start, end - start + 1);
        } else
            szTemp = strdup("");
    } else {
        end = p;
        if (end > start) {
            szTemp = strndup(s + start, end - start);
        } else
            szTemp = strdup("");
    }
    m_put(m, &szTemp);

    if (s[p])
      p++;
    else
      break;
  }

  return m;
}

#include <regex.h>

/**
 * @brief Matches a regular expression against a string and returns captured groups.
 *
 * @param m A string list handle or 0.
 * @param regex The regular expression.
 * @param s The string to search.
 * @return A string list handle containing the matches.
 */
int m_regex(int m, const char *regex, const char *s) {
  char *szTemp;
  regex_t regc;
  regmatch_t *pm;
  int i, error;

  int subexp = 1;
  int p = 0;

  error = regcomp(&regc, regex, REG_EXTENDED);
  if (error)
    ERR("REG_EXPRESSION %s not valid", regex);

  while (regex[p]) {
    if (regex[p] == '(')
      subexp++;
    p++;
  }
  pm = (regmatch_t *)malloc(sizeof(regmatch_t) * subexp);

  if (m > 1)
	  m_free_strings(m, 1);
  else
	  m = m_alloc(subexp + 1, sizeof(char *), MFREE_STR);

  error = regexec(&regc, s, subexp, pm, 0);
  if (!error) {
    for (i = 0; i < subexp; i++) {
      if (pm[i].rm_so == -1)
        break;
      szTemp = strndup(s + pm[i].rm_so, pm[i].rm_eo - pm[i].rm_so);
      m_put(m, &szTemp);
    }
  }
  free(pm);
  regfree(&regc);
  return m;
}

/**
 * @brief Creates a duplicate of a list.
 *
 * @param m List handle to duplicate.
 * @return New list handle.
 */
int m_dub(int m) {
	int h = m_free_hdl( m );
	int r = m_alloc(m_len(m), m_width(m), h);  
	m_write(r, 0, mls(m, 0), m_len(m));  
	return r;
}

/*
0xxxxxxx
10111111   illegal
110xxxxx   10xxxxxx
1110xxxx   10xxxxxx 10xxxxxx
11110xxx   10xxxxxx 10xxxxxx 10xxxxxx
*/
#define UTF8GET()                                                              \
  if (EOS())                                                                   \
    return -1;                                                                 \
  c = GETCH();                                                                 \
  INC();                                                                       \
                                                                               \
  if ((c & 0x80) == 0)                                                         \
    return c;                                                                  \
  if ((c & 0x40) == 0)                                                         \
    return 0xFFFD;                                                             \
  if ((c & 0x20) == 0) {                                                       \
    len = 1;                                                                   \
    c &= 0b00011111;                                                           \
    goto read;                                                                 \
  }                                                                            \
  if ((c & 0x10) == 0) {                                                       \
    len = 2;                                                                   \
    c &= 0b00001111;                                                           \
    goto read;                                                                 \
  }                                                                            \
  if ((c & 0x08) == 0) {                                                       \
    len = 3;                                                                   \
    c &= 0b00000111;                                                           \
    goto read;                                                                 \
  }                                                                            \
  if ((c & 0x04) == 0) {                                                       \
    len = 4;                                                                   \
    c &= 0b00000011;                                                           \
    goto read;                                                                 \
  }                                                                            \
  if ((c & 0x02) == 0) {                                                       \
    len = 5;                                                                   \
    c &= 0b00000001;                                                           \
    goto read;                                                                 \
  }                                                                            \
  return 0xFFFD;                                                               \
                                                                               \
  read:                                                                        \
  ret = c;                                                                     \
  while (len > 0) {                                                            \
    len--;                                                                     \
    if (EOS())                                                                 \
      return -1;                                                               \
    c = GETCH();                                                               \
    if ((c & 0xc0) != 0x80) /* wrong header */                                 \
      return 0xFFFD;                                                           \
    INC();                                                                     \
    ret = (ret << 6) | (c & 0x3f);                                             \
  }                                                                            \
                                                                               \
  return ret

/**
 * @brief Decodes a UTF-8 character from a list buffer.
 *
 * @param buf List handle.
 * @param p Pointer to byte offset in buffer. Incremented by character length.
 * @return 32-bit character code, or -1 on EOS, or 0xFFFD on error.
 */
int m_utf8char(int buf, int *p) {
  unsigned char c;
  uint32_t ret;
  int len;

#define GETCH() (*(unsigned char *)mls(buf, (*p)))
#define EOS() ((*p) >= m_len(buf))
#define INC() ((*p)++)

  UTF8GET();

#undef GETCH
#undef EOS
#undef INC
}

/**
 * @brief Decodes a UTF-8 character from a string.
 *
 * @param s Pointer to string pointer. Incremented by character length.
 * @return 32-bit character code, or -1 on EOS, or 0xFFFD on error.
 */
int utf8char(char **s) {
  unsigned char c;
  uint32_t ret;
  int len;

#define GETCH() (**s)
#define EOS() ((**s) == 0)
#define INC() ((*s)++)

  UTF8GET();

#undef GETCH
#undef EOS
#undef INC
}

/**
 * @brief Reads a UTF-8 character from a file.
 *
 * @param fp File pointer.
 * @param buf Buffer to store raw bytes (buf[0] is length).
 * @return First byte of the character, or -1 on error.
 */
int utf8_getchar(FILE *fp, utf8_char_t buf) {
  int len, ch, nx, i;

read_single:
  ch = fgetc(fp);

parse_next_char:
  if (ch < 0x80) // valid char or EOF, Lower Than instead of Logic AND
  {
    len = 1;
    goto read_multi_byte;
  }

  if ((ch & 0x40) == 0) // invalid, discard
  {
    goto read_single;
  }
  if ((ch & 0x20) == 0) // Bit 7=0
  {
    len = 2;
    goto read_multi_byte;
  }

  if ((ch & 0x10) == 0) // Bit 6=0
  {
    len = 3;
    goto read_multi_byte;
  }

  if ((ch & 0x08) == 0) // 4 byte
  {
    len = 4;
    goto read_multi_byte;
  }

  if ((ch & 0x04) == 0) // 5 byte
  {
    len = 5;
    goto read_multi_byte;
  }

  if ((ch & 0x02) == 0) // 6 byte
  {
    len = 6;
    goto read_multi_byte;
  }

  // illegal char, read next
  goto read_single;

read_multi_byte:
  buf[0] = len;
  buf[1] = ch < 0 ? 0xff : ch;
  i = 1;
  while (i++ < len) {
    nx = fgetc(fp);
    if (nx < 0)
      return -1;
    if ((nx & 0xc0) != 0x80) { // wrong header, discard char
      ch = nx;
      goto parse_next_char;
    }
    buf[i] = nx;
  }
  return ch;
}

/**
 * @brief Scans a file until a delimiter is found.
 *
 * @param m List handle to store results.
 * @param delim Delimiter character.
 * @param fp File pointer.
 * @return Delimiter character or -1 on EOF.
 */
int m_fscan(int m, char delim, FILE *fp) {
  int ch;
  utf8_char_t buf;
  buf[0] = 0;
  for (;;) {
    ch = utf8_getchar(fp, buf);
    if (ch < 0 || ch == delim) {
      if (m) {
        buf[0] = 0;
        m_put(m, buf);
      }
      return ch;
    }
    if (m)
      m_write(m, m_len(m), buf + 1, *buf);
  }
}

/**
 * @brief Scans a file with whitespace reduction.
 *
 * @param m List handle.
 * @param delim Delimiter.
 * @param fp File pointer.
 * @return Delimiter or -1 on EOF.
 */
int m_fscan2(int m, char delim, FILE *fp) {
  int ch;
  int IN = 1;
  char SPACE = 0;
  utf8_char_t buf;
  buf[0] = 0;
  for (;;) {
    ch = utf8_getchar(fp, buf);
    if (ch < 0 || ch == delim) {
      if (m) {
        buf[0] = 0;
        m_put(m, buf);
      }
      return ch;
    }
    if (m) {
      if (isspace(ch)) {
        if (IN)
          continue;
        if (SPACE)
          continue;
        SPACE = 32;
        continue;
      } else {
        if (IN)
          IN = 0;
        if (SPACE && m)
          m_put(m, &SPACE);
        SPACE = 0;
      }
      m_write(m, m_len(m), buf + 1, *buf);
    }
  }
}

/**
 * @brief Compares two string lists using strncmp.
 *
 * @param a First list handle.
 * @param b Second list handle.
 * @return Comparison result.
 */
int m_cmp(int a, int b) {
  int l1, l2;
  l1 = m_len(a);
  l2 = m_len(b);
  l1 = Min(l1, l2);
  return strncmp((char *)mls(a, 0), (char *)mls(b, 0), l1);
}

/**
 * @brief Searches for an object in a list, adding it if not found.
 *
 * @param m List handle.
 * @param obj Pointer to object.
 * @param size Object size.
 * @return Index of the object.
 */
int m_lookup_obj(int m, void *obj, int size) {
  int p;
  void *d;

  m_foreach(m, p, d) if (memcmp(d, obj, size) == 0) return p;

  p = m_new(m, 1);
  memcpy(mls(m, p), obj, size);
  return p;
}

/**
 * @brief Lookup table management for string handles.
 *
 * @param m List handle.
 * @param key String handle to look up.
 * @return Existing string handle or key if newly inserted.
 */
int m_lookup(int m, int key) {
  int p, *d;

  if (m_len(key) == 0)
    ERR("Key of zero size");
  m_foreach(m, p, d) if (m_cmp(*d, key) == 0) return *d;

  m_put(m, &key);
  return key;
}

/**
 * @brief Searches for a C-string in a list, optionally adding it.
 *
 * @param m List handle.
 * @param key C-string to look up.
 * @param NOT_INSERT If non-zero, do not insert if not found.
 * @return Index of the string, or -1 if not found and NOT_INSERT is set.
 */
int m_lookup_str(int m, const char *key, int NOT_INSERT) {
  int p;
  char **d;

  if (!key || strlen(key) == 0)
    ERR("Key of zero size");
  m_foreach(m, p, d) {
    if (*d == NULL)
      continue;
    if (strcmp(*d, key) == 0)
      return p;
  }

  if (NOT_INSERT)
    return -1;

  p = m_new(m, 1);
  *(char **)mls(m, p) = strdup(key);
  return p;
}

/**
 * @brief Integer comparison function for qsort/bsearch.
 */
int
cmp_int(const void *a0, const void *b0)
{
	const int *a = a0;
	const int *b = b0;
	return (*a) - (*b);
}

/**
 * @brief Inserts an integer into a sorted list.
 */
int
m_binsert_int(int buf, int key)
{
	return m_blookup_int(buf,key,NULL,NULL);
}

/**
 * @brief Searches for an integer in a sorted list.
 */
int
m_bsearch_int(int buf, int key)
{
	return m_bsearch(&key, buf, cmp_int);
}

/**
 * @brief Binary lookup for an integer, inserting if not found and calling an initializer.
 *
 * @param buf List handle.
 * @param key Integer key.
 * @param new Initializer function for new entries.
 * @param ctx Context for initializer.
 * @return Index of the entry.
 */
int
m_blookup_int(int buf, int key, void (*new)(void *, void *), void *ctx)
{
	void *obj = calloc(1, m_width(buf));
	*(int *)obj = key;
	int p = m_binsert(buf, obj, cmp_int, 0);
	free(obj);
	if (p < 0) { /* entry exists */
		return (-p) - 1;
	}
	if (new)
		new (mls(buf, p), ctx);
	return p;
}

/**
 * @brief Same as m_blookup_int but returns a pointer to the element.
 */
void*   m_blookup_int_p(int buf, int key, void (*new)(void *, void *), void *ctx)
{
	return mls(buf, m_blookup_int(buf,key,new,ctx));	
}






/**
 * @brief Initializes a new set of variables.
 *
 * @return Handle of the variable list.
 */
int v_init(void) { return m_create(100, sizeof(int)); }

/**
 * @brief Frees a set of variables and their associated string lists.
 *
 * @param vl Handle of the variable list.
 */
void v_free(int vl) {
  int p, *d;
  m_foreach(vl, p, d) m_free_strings(*d, 0);
  m_free(vl);
}

/**
 * @brief Sets a variable's value.
 *
 * @param vs Handle of the variable set.
 * @param name Variable name.
 * @param value Variable value.
 * @param pos Position in the value list (VAR_APPEND for end).
 * @return Handle of the variable's value list.
 */
int v_set(int vs, const char *name, const char *value, int pos) {
  int key = v_lookup(vs, name);
  v_kset(key, value, pos);
  return key;
}

/**
 * @brief Sets multiple variables from a variadic list.
 *
 * @param vs Handle of the variable set.
 * @param ... Pairs of char *name, char *value, terminated by NULL.
 */
void v_vaset(int vs, ...) {
  va_list argptr;
  char *name, *value;

  va_start(argptr, vs);

  while ((name = va_arg(argptr, char *)) != NULL) {
    value = va_arg(argptr, char *);
    v_set(vs, name, value, VAR_APPEND);
  }

  va_end(argptr);
}

/**
 * @brief Clears a variable's value list.
 *
 * @param vs Handle of the variable set.
 * @param name Variable name.
 */
void v_clr(int vs, const char *name) { v_kclr(v_lookup(vs, name)); }

/**
 * @brief Retrieves a variable's value.
 *
 * @param vs Handle of the variable set.
 * @param name Variable name.
 * @param pos Position in the value list.
 * @return Variable value as C-string.
 */
char *v_get(int vs, const char *name, int pos) {
  return v_kget(v_lookup(vs, name), pos);
}

/**
 * @brief Returns the number of values for a variable.
 *
 * @param vs Handle of the variable set.
 * @param name Variable name.
 * @return Number of values.
 */
int v_len(int vs, const char *name) { return v_klen(v_lookup(vs, name)); }

/**
 * @brief Finds the index of a variable by name.
 *
 * @param vs Handle of the variable set.
 * @param name Variable name.
 * @return Index or -1 if not found.
 */
int v_find_key(int vs, const char *name) {
  char *s;
  int p, *d;
  m_foreach(vs, p, d) {
    s = STR(*d, 0);
    if (strcmp(s, name) == 0)
      return p;
  }
  return -1;
}

/**
 * @brief Removes a variable from a set.
 *
 * @param vs Handle of the variable set.
 * @param name Variable name.
 */
void v_remove(int vs, const char *name) {
  int pos = v_find_key(vs, name);
  if (pos >= 0) {
    m_free_strings(INT(vs, pos), 0);
    m_del(vs, pos);
  }
}

/**
 * @brief Retrieves the handle of a variable's value list, creating it if needed.
 *
 * @param vl Handle of the variable set.
 * @param name Variable name.
 * @return Handle of the variable's value list.
 */
int v_lookup(int vl, const char *name) {
  if (is_empty(name))
    return -1;

  int p = v_find_key(vl, name);
  if (p >= 0) {
    return INT(vl, p);
  }

  TRACE(1, "Create on Stack (%d) Var: %s", vl, name);
  int var = m_create(2, sizeof(char *));
  char *s = strdup(name);
  m_put(var, &s);
  m_put(vl, &var);
  return var;
}

/**
 * @brief Sets a value in a variable's value list.
 *
 * @param var Handle of the variable's value list.
 * @param v Value string.
 * @param row Index in the list (or VAR_APPEND).
 */
void v_kset(int var, const char *v, int row) {
  char *val = NULL;
  if (v)
    val = strdup(v);

  if (row < 0 || row >= m_len(var)) // append-value
  {
    m_put(var, &val);
  } else // replace value
  {
    char **d = (char **)mls(var, row);
    if (*d)
      free(*d);
    *d = val;
  }
}

/**
 * @brief Clears all values for a variable but keeps its name.
 *
 * @param var Handle of the variable's value list.
 */
void v_kclr(int var) {
  int i = 0;
  char **d;
  while (m_next(var, &i, &d))
    if (*d) {
      free(*d);
      *d = NULL;
    }

  m_setlen(var, 1);
}

/**
 * @brief Retrieves a value from a variable's value list by index.
 *
 * @param var Handle of the variable's value list.
 * @param row Index.
 * @return Value string.
 */
char *v_kget(int var, int row) {
  if (var <= 0)
    return "";
  int len = m_len(var);
  if (row >= len)
    return "";
  char *s = STR(var, row);
  if (s == NULL)
    return "";
  return s;
}

/**
 * @brief Returns the number of values in a variable's value list (excluding name).
 */
int v_klen(int key) { return m_len(key) - 1; }

/**
 * @brief Initializes a string expansion structure.
 */
void se_init(str_exp_t *se) { memset(se, 0, sizeof *se); }

/**
 * @brief Frees a string expansion structure and its buffers.
 */
void se_free(str_exp_t *se) {
  m_free_strings(se->splitbuf, 0);
  m_free(se->values);
  m_free(se->indices);
  m_free(se->buf);
  memset(se, 0, sizeof *se);
}

/**
 * @brief Reallocates or clears buffers in a string expansion structure.
 */
void se_realloc_buffers(str_exp_t *se) {
  if (!se->buf) {
    se->splitbuf = m_create(10, sizeof(char *));
    se->values = m_create(10, sizeof(char *));
    se->indices = m_create(10, sizeof(int));
    se->buf = m_create(100, 1);
    return;
  }

  m_free_strings(se->splitbuf, 1);
  m_clear(se->values);
  m_clear(se->indices);
  m_clear(se->buf);
  se->max_row = 0;
}

/**
 * @brief Internal function to parse a variable index in a template string.
 *
 * @param s Pointer to current position in template string.
 * @return Parsed index code.
 */
static int parse_index(const char **s) {
  int val;
  const char *p = *s;

  if (*p != '[')
    return 0;
  p++;
  if (*p == '*' && p[1] == ']') {
    *s = p + 2;
    return 1;
  }

  val = 0;
  while (isdigit(*p)) {
    val *= 10;
    val += *p - '0';
    p++;
  }
  if (*p == ']') {
    *s = p + 1;
    return val + 2;
  }

  return 0;
}

/**
 * @brief Parses a template string for expansion.
 *
 * @param se String expansion structure.
 * @param frm Template string.
 */
void se_parse(str_exp_t *se, const char *frm) {
  ASSERT(frm && se);

  se_realloc_buffers(se); // alloc, or clear buffer

  int b = se->splitbuf;
  char *cp, prev;
  const char *s, *s0;

  prev = 0;
  s = frm;
  s0 = s;

  for (;;) {

    if (*s == 0 || (*s == '$' && prev != '\\')) {
      // prefix ?
      if (s0 != s) {
        cp = strndup(s0, s - s0); // copy without *s
        m_put(b, &cp);
      }
      if (*s == 0)
        break; // exit

      // cut out varname
      s0 = s; //  token start (with $-prefix)
      s++;    // skip leading $

      if (*s == '\'') {
        s++;
      } // expand with single quotes

      while (isalnum(*s) || *s == '_')
        s++; // UNTIL DELIMITER FOUND
      // copy without delimiter
      cp = strndup(s0, s - s0);
      m_put(b, &cp);
      m_put(se->values, &cp);

      int index = parse_index(&s);
      m_put(se->indices, &index);
      if (*s == 0)
        break; // exit

      s0 = s; // s0 points to delimiter
    }
    prev = *s;
    s++;
  }
};

/**
 * @brief Replaces a single character with its escaped version if necessary.
 */
static void repl_char(int buf, char ch) {
  char tab[] = {'\\', '\0', '\n', '\r', '\'', '"', '\x1a'};
  char rep[] = {'\\', '0', 'n', 'r', '\'', '"', 'Z'};

  int i;
  for (i = 0; i < sizeof tab; i++)
    if (tab[i] == ch) {
      m_putc(buf, '\\');
      m_putc(buf, rep[i]);
      return;
    }
  m_putc(buf, ch);
}

/**
 * @brief Escapes characters in a string and appends to a list buffer.
 */
void escape_buf(int buf, char *src) {
  while (*src)
    repl_char(buf, *src++);
}

/**
 * @brief Escapes a string and returns it in a new list buffer.
 */
int escape_str(int buf, char *src) {
  if (!buf)
    buf = m_create(100, 1);
  else
    m_clear(buf);
  escape_buf(buf, src);
  m_putc(buf, 0);
  return buf;
}

/**
 * @brief Internal function to escape a field, optionally with quotes.
 */
static int field_escape(int s2, char *s, int quotes) {
  // "*s" ist der zu speichernde string
  // um das sql-kommando zu generieren werden sonderzeichen
  // "escaped". dies ist ein gutes beispiel warum die "mls"
  // speicherverwaltung vorteile bietet. der benötigte speicher
  // von mysql_escape_string muss abgeschätzt und reserviert werden.
  // die gleiche funktion in mls ist viel einfacher zu verwenden
  if (quotes)
    m_putc(s2, '\'');
  escape_buf(s2, s);
  if (quotes)
    m_putc(s2, '\'');
  return s2;
}

/**
 * @brief Expands a parsed template with variables.
 *
 * @param se Parsed template.
 * @param vl Variable set.
 * @param row Default row index for variable expansion.
 * @return Expanded string.
 */
char *se_expand(str_exp_t *se, int vl, int row) {
  int var, index;
  int p, vn;
  char **d, *s;
  int quotes = 0;
  m_clear(se->buf);
  int buf = se->buf;
  vn = 0; // number of variables

  // string zusammenfügen
  // variablen werden durch ihren wert ersetzt
  // variablen werden durch ein führendes "$" erkannt
  // folgt dem $ ein "'" wird der eingesetzte wert durch "'" umschlossen
  //
  m_foreach(se->splitbuf, p, d) {
    s = *d;

    if (*s != '$') { // einfacher text-baustein, nur anhängen
      m_write(buf, m_len(buf), s, strlen(s));
    } else // variable found
    {
      if (s[1] == '\'') {
        quotes = 1;
        s++;
      } else
        quotes = 0;
      var = v_lookup(vl, s + 1);
      index = INT(se->indices, vn);
      vn++;

      // expand var
      if (index == 1) { // erzeuge eine liste von werten
        field_escape(buf, STR(var, 1), quotes);
        for (index = 2; index < m_len(var); index++) {
          m_putc(buf, ',');
          field_escape(buf, STR(var, index), quotes);
        }
      } else { // index != 1  i.e. not expand all i.e. index != [*]
        if (index == 0)
          index = row; // falls kein index angegeben wurde, benutze (row)
        else
          index -= 2;

        if (index < v_klen(var))
          field_escape(buf, STR(var, index + 1), quotes);
      }
    } // variable expandiert
  }

  m_putc(buf, 0);
  return mls(buf, 0);
}

/**
 * @brief Parses and expands a template string in one go.
 *
 * @param vl Variable set handle.
 * @param frm Template string.
 * @return Expanded string.
 */
char *se_string(int vl, const char *frm) {
  str_exp_t se;
  int data;

  se_init(&se);
  se_parse(&se, frm);
  se_expand(&se, vl, 0);
  data = v_set(vl, "se_string", mls(se.buf, 0), 1);
  se_free(&se);
  return STR(data, 1);
}

/**
 * @brief Returns the length of a string in a list buffer, excluding terminating zero if present.
 */
int s_strlen(int m) {
  int p = m_len(m);
  return p && CHAR(m, p - 1) == 0 ? p - 1 : p;
}

/**
 * @brief Appends a C-string to a string list buffer.
 */
int s_app1(int m, char *s) {
  int p = s_strlen(m);
  m_write(m, p, s, strlen(s) + 1);
  return m;
}

/**
 * @brief Internal function for variadic string append.
 */
static int vas_app(int m, va_list ap) {
  char *name;
  while ((name = va_arg(ap, char *)) != NULL) {
    s_app1(m, name);
  }
  return m;
}

/**
 * @brief Appends multiple C-strings to a list buffer.
 */
int s_app(int m, ...) {
  va_list ap;
  if (!m)
    m = m_create(10, 1);
  va_start(ap, m);
  vas_app(m, ap);
  va_end(ap);
  return m;
}

/**
 * @brief Formatted print to a list buffer.
 *
 * @param m List handle (if 0, a new list is created).
 * @param p Position (if <0 or >len, appends).
 * @param format Printf-style format string.
 * @param ap Variadic argument list.
 * @return List handle.
 */
int vas_printf(int m, int p, const char *format, va_list ap) {
  int len;
  va_list copy;

  // Patch für 64Bit machines 08.10.14
  va_copy(copy, ap);

  len = vsnprintf(0, 0, format, ap); /* get string size */
  len++;                             /* with terminating zero */
  if (m == 0) {
    m = m_create(len, 1);
    p = 0;
  }

  if (p < 0 || p > m_len(m)) /* append to string */
    p = s_strlen(m);

  m_setlen(m, p + len);
  void *buf = mls(m, p);

  vsnprintf(buf, len, format, copy); /* len is (stringsize + 1) */
  va_end(copy);
  return m;
}

/**
 * @brief Variadic formatted print to a list buffer.
 */
int s_printf(int m, int p, char *format, ...) {
  va_list ap;
  va_start(ap, format);
  m = vas_printf(m, p, format, ap);
  va_end(ap);
  return m;
}

/**
 * @brief Finds the last non-zero character in a string list buffer.
 */
int s_lastchar(int m) {
  int len = m_len(m);
  if (len == 0)
    return 0;

  do {
    len--;
  } while (len > 0 && CHAR(m, len) == 0);

  return CHAR(m, len);
}

/**
 * @brief Creates a copy of a substring.
 *
 * @param m List handle.
 * @param first_char Starting index.
 * @param last_char Ending index (or -1 for end of string).
 * @return New list handle.
 */
int s_copy(int m, int first_char, int last_char) {
  if (last_char < 0)
    last_char = m_len(m) - 1;
  if (first_char < 0 || first_char > last_char || first_char >= m_len(m))
    return m_create(1, 1);
  int size = last_char - first_char + 1;
  if (first_char + size > m_len(m))
    size = m_len(m) - first_char;
  int ret = m_create(size, 1);
  m_write(ret, 0, mls(m, first_char), size);
  if (CHAR(ret, m_len(ret) - 1) != 0)
    m_putc(ret, 0);
  return ret;
}

/**
 * @brief Standard qsort wrapper for MLS lists.
 */
void m_qsort(int list, int (*compar)(const void *, const void *)) {
  qsort(m_buf(list), m_len(list), m_width(list), compar);
}

/**
 * @brief Standard bsearch wrapper for MLS lists.
 *
 * @return Index of found element, or -1.
 */
int m_bsearch(const void *key, int list,
              int (*compar)(const void *, const void *)) {
  if (list < 1 || m_len(list) == 0)
    return -1;
  void *res = bsearch(key, m_buf(list), m_len(list), m_width(list), compar);
  if (res)
    return (res - m_buf(list)) / m_width(list);
  return -1;
}

/**
 * @brief Standard lfind wrapper for MLS lists.
 */
int m_lfind(const void *key, int list,
            int (*compar)(const void *, const void *)) {
  size_t max;
  if (list < 1 || m_len(list) == 0)
    return -1;
  max = m_len(list);
  void *res = lfind(key, m_buf(list), &max, m_width(list), compar);
  if (res)
    return (res - m_buf(list)) / m_width(list);
  return -1;
}

/**
 * @brief Inserts data into a sorted list.
 *
 * @param buf List handle.
 * @param data Pointer to data.
 * @param cmpf Comparison function.
 * @param with_duplicates If non-zero, allow duplicates.
 * @return Position of new element, or (-pos-1) if element exists and duplicates are not allowed.
 */
int m_binsert(int buf, const void *data,
              int (*cmpf)(const void *data, const void *buf_elem),
              int with_duplicates) {
  int left = 0;
  int right = m_len(buf) + 1;
  int cur = 1;
  void *obj;
  int cmp;

  if (m_len(buf) == 0) {
    m_put(buf, data);
    return 0;
  }

  while (1) {
    cur = (left + right) / 2;
    obj = mls(buf, cur - 1);
    cmp = cmpf(data, obj);
    if (cmp == 0) {
      if (!with_duplicates)
        return -cur;
      break;
    }
    if (cmp < 0) {
      right = cur;
      if (left + 1 == right)
        break;
    } else {
      left = cur;
      if (left + 1 == right) {
        cur++;
        break;
      }
    }
  }

  cur--;
  m_ins(buf, cur, 1);
  m_write(buf, cur, data, 1);
  return cur;
}

/**
 * @brief Searches for a byte character in a list buffer.
 *
 * @param buf List handle.
 * @param p Starting index.
 * @param ch Character code.
 * @return Index or -1 if not found.
 */
int s_index(int buf, int p, int ch) {
  unsigned char *d;
  while (p < m_len(buf)) {
    d = mls(buf, p);
    if (*d == ch)
      return p;
    p++;
  }
  return -1;
}

/**
 * @brief Exported version of _get_list for external modules.
 */
lst_t *exported_get_list(int r) { return _get_list(r); }

/**
 * @brief Creates a ring buffer.
 *
 * @param size Buffer capacity.
 * @return Ring buffer handle.
 */
int ring_create(int size) {
  int r = m_create(size + 1, sizeof(int));
  lst_t *lp = _get_list(r);
  int *rd = lst_peek(*lp, 0);
  int *wr = &(*lp)->l;
  *rd = -1;
  *wr = 1;
  return r;
}

/**
 * @brief Checks if a ring buffer is empty.
 */
int ring_empty(int r) {
  lst_t *lp = _get_list(r);
  int *rd = lst_peek(*lp, 0);
  return (*rd < 0);
}

/**
 * @brief Checks if a ring buffer is full.
 */
int ring_full(int r) {
  lst_t *lp = _get_list(r);
  int *rd = lst_peek(*lp, 0);
  int *wr = &(*lp)->l;
  return (*rd == *wr);
}

/**
 * @brief Puts an integer into a ring buffer.
 *
 * @return 0 on success, -1 if full.
 */
int ring_put(int r, int data) {
  lst_t *lp = _get_list(r);
  int *rd = lst_peek(*lp, 0);
  int *wr = &(*lp)->l;
  int max = (*lp)->max;
  int *d = lst_peek(*lp, *wr);

  if (*rd == *wr)
    return -1; /* full */
  *d = data;
  if (*rd < 0)
    *rd = *wr; /* was empty */
  (*wr)++;
  if (*wr >= max)
    *wr = 1;
  return 0;
}

/**
 * @brief Gets an integer from a ring buffer.
 *
 * @return Value or -1 if empty.
 */
int ring_get(int r) {
  lst_t *lp = _get_list(r);
  int *rd = lst_peek(*lp, 0);
  int *wr = &(*lp)->l;
  int max = (*lp)->max;

  if (*rd < 0)
    return -1; /* empty */
  int *d = lst_peek(*lp, *rd);
  (*rd)++;
  if (*rd >= max)
    *rd = 1;
  if (*rd == *wr)
    *rd = -1;
  return *d;
}

/**
 * @brief Frees a ring buffer.
 */
void ring_free(int r) { m_free(r); }

/**
 * @brief Compares a list buffer string with a C-string.
 *
 * @param m List handle.
 * @param p Starting index in list.
 * @param s C-string.
 * @return 0 if equal, otherwise non-zero.
 */
int mstrcmp(int m, int p, const char *s) {
  int res = 1;
  if (!s)
    return 1;
  while (p < m_len(m)) {
    res = CHAR(m, p) - *s;
    if (res)
      break;
    if (*s == 0)
      return 0;
    p++;
    s++;
  }
  return res;
}

/**
 * @brief Converts a string in a list buffer to a long integer.
 *
 * @param buf List handle.
 * @param p Pointer to starting index.
 * @param ret_val Pointer to receive converted value.
 * @return 0 on success, -1 on error.
 */
int mstr_to_long(int buf, int *p, long int *ret_val) {

  int pp = 0;
  if (!p)
    p = &pp;
  if (buf <= 0 || *p < 0 || *p >= m_len(buf))
    return -1;

  /* append zero but keep array length */
  if (CHAR(buf, m_len(buf) - 1) != 0) {
    m_putc(buf, 0);
    m_setlen(buf, m_len(buf) - 1);
  }

  errno = 0;
  char *endptr;
  char *start = mls(buf, *p);
  *ret_val = strtol(start, &endptr, 0);
  if (*endptr || errno)
    return -1;
  return 0;
}

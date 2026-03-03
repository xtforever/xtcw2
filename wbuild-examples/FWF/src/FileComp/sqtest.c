#include <stdio.h>

main(argc, argv)
     int	argc;
     char	**argv;
{
  int	i;
  for (i=1; i<argc; i++) {
    char	buf[256];
    strcpy(buf, argv[i]);
    squish_filespec(buf);
    printf("%s\n", buf);
  }
}

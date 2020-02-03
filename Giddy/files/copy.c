#include <stdio.h>
#include <stdlib.h>
 
int main()
{
   char ch;
   FILE *source, *target;
   char source_file[]="C:\\Users\\Administrator\\Desktop\\root.txt";
   char target_file[]="C:\\Users\\stacy\\Desktop\\root.txt";
   source = fopen(source_file, "r");
   if (source == NULL)
   {
      printf("Press any key to exit...\n");
      exit(EXIT_FAILURE);
   }
   target = fopen(target_file, "w");
   if (target == NULL)
   {
      fclose(source);
      printf("Press any key to exit...\n");
      exit(EXIT_FAILURE);
   }

   while ((ch = fgetc(source)) != EOF)
      fputc(ch, target);
     
   printf("File copied successfully.\n");
     
   fclose(source);
   close(target);
   return 0;
}

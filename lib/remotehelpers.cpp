

/**
 * Get the IP address
 * @return char *
 */
const char * getIpAddress(){
  static char w[128];
  FILE *f=popen("ip a | grep 'scope global' | grep -v ':' | awk '{print $2}' | cut -d '/' -f1","r");
  int c,i=0;
  while((c=getc(f))!=EOF)i+=sprintf(w+i,"%c",c);
  pclose(f);
  return w;
}



/**
 * Get the IP address
 * @return char *
 */
const char * getIpAddress(){
  static char w[128];
  FILE *f=popen("ip a | grep 'scope global' | grep -v ':' | awk '{print $2}' | cut -d '/' -f1","r");
  int c,i=0;
  while((c=getc(f))!=EOF && i < (int)sizeof(w)-1) w[i++]=(char)c;
  w[i]='\0';
  pclose(f);
  return w;
}

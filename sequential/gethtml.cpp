#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <curl/curl.h>
//#include <curl/types.h>
#include <curl/easy.h>

//code from http://www.cplusplus.com/forum/windows/36638/
// also very useful https://curl.se/libcurl/c/htmltitle.html

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
  int written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

int main(int argc, char *argv[])
{
  CURL *curl_handle;
  static const char *htmlfilename = "html_code.html";
  FILE *htmlfile;

  if(argc < 2) {
    printf("Usage: %s <URL>\n", argv[0]);
    return 1;
  }

  curl_global_init(CURL_GLOBAL_ALL);

  /* init the curl session */
  curl_handle = curl_easy_init();

  /* set URL to get */
  curl_easy_setopt(curl_handle, CURLOPT_URL, argv[1]);

  //follow redirects automatically
  curl_easy_setopt(curl_handle,CURLOPT_FOLLOWLOCATION,1);

  // /* Switch on full protocol/debug output while testing */
  // curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);

  /* no progress meter please */
  curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);

  /* send all data to this function  */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

  /* open the files */
  htmlfile = fopen(htmlfilename,"w");
  if (htmlfile == NULL) {
    curl_easy_cleanup(curl_handle);
    printf("error: html not obtained\n");
    return -1;
  }

  printf("success: html obtained\n");

  /* we want the headers to this file handle */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, htmlfile);

  /* get it! */
  curl_easy_perform(curl_handle);

  /* close the header file */
  fclose(htmlfile);

  /* cleanup curl stuff */
  curl_easy_cleanup(curl_handle);

  curl_global_cleanup();

  return 0;
}
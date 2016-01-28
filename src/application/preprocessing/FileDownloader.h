//
//#ifndef FILEDOWNLOADER_H_
//#define FILEDOWNLOADER_H_
//
//#include <stdio.h>
//#include <stdlib.h>
//#include <unistd.h>
//#include <curl/curl.h>
//
//class FileDownloader {
//
//private:
//	static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
//	{
//	  size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
//	  return written;
//	}
//
//
//public:
//
//	static bool downloadFile(std::string url, std::string outputFileName)
//	{
//		std::cout << "\nBegin downloading file at " << url << std::endl;
//		CURL *curl_handle;
//		FILE *pagefile;
//
//		curl_global_init(CURL_GLOBAL_ALL);
//
//		/* init the curl session */
//		curl_handle = curl_easy_init();
//
//		/* set URL to get here */
//		curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
//
//		/* Switch on full protocol/debug output while testing */
//		curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 0L);
//
//		/* disable progress meter, set to 0L to enable and disable debug output */
//		curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
//
//		/* send all data to this function  */
//		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
//
//		/* open the file */
//		pagefile = fopen(outputFileName.c_str(), "wb");
//		if (pagefile) {
//
//			/* write the page body to this file handle. CURLOPT_FILE is also known as
//		       CURLOPT_WRITEDATA*/
//			curl_easy_setopt(curl_handle, CURLOPT_FILE, pagefile);
//
//			/* get it! */
//			curl_easy_perform(curl_handle);
//
//			/* close the header file */
//			fclose(pagefile);
//		}
//
//		/* cleanup curl stuff */
//		curl_easy_cleanup(curl_handle);
//
//		std::cout << "Downloading complete\n\n";
//
//		return 0;
//	}
//
//
//
//
//
//};
//
//#endif

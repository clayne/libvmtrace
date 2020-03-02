
#include <util/ElasticLogger.hpp>

namespace libvmtrace
{
namespace util
{
namespace
{
	size_t callback_dont_care(const char* in, size_t size, size_t num, std::string* out)
	{
		return size * num;
	}

	size_t callback_string(const char* in, size_t size, size_t num, std::string* out)
	{
		const size_t totalBytes(size * num);
		out->append(in, totalBytes);
		return totalBytes;
	}
}

	bool ElasticLogger::CheckIndex(const std::string index)
	{
		if(!_ok)
			return false;

		_curl = curl_easy_init();

		//curl_easy_setopt(_curl, CURLOPT_POSTFIELDS, "{\"hi\" : \"there\"}");
		std::string url = _base_url+index+"/";
		curl_easy_setopt(_curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, callback_dont_care);
		_res = curl_easy_perform(_curl);

		long http_code = 0;
		curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &http_code);

		curl_easy_cleanup(_curl);

		if(http_code == 200)
			return true;

		return false;
	}

	bool ElasticLogger::CreateIndex(const std::string index, const std::string mappings)
	{
		if(!_ok)
			return false;

		const std::string payload = mappings == "" ? "{ \"settings\" : { \"index\" : { } } }" : "{ "+mappings+" ,\"settings\" : { \"index\" : { } } }";

		_curl = curl_easy_init();

		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, "Accept: application/json");
		headers = curl_slist_append(headers, "Content-Type: application/json");
		headers = curl_slist_append(headers, "charsets: utf-8");

		std::string url = _base_url+index+"/";
		curl_easy_setopt(_curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(_curl, CURLOPT_CUSTOMREQUEST, "PUT");
		curl_easy_setopt(_curl, CURLOPT_POSTFIELDS, payload.c_str());
		curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, callback_dont_care);
		curl_easy_setopt(_curl, CURLOPT_POSTFIELDSIZE, strlen(payload.c_str()));
		_res = curl_easy_perform(_curl);

		long http_code = 0;
		curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &http_code);

		curl_easy_cleanup(_curl);

		if(http_code == 200)
			return true;
		
		return false;
	}

	std::string ElasticLogger::Query(const std::string index, const std::string query, const std::string size, const std::string sort)
	{
		_curl = curl_easy_init();

		std::unique_ptr<std::string> http_response(new std::string());

		std::string url = _base_url+index+"/_search?size="+size+"&sort="+sort+"&q=";
		char* encoded_query = curl_easy_escape(_curl, query.c_str(), 0);
		// cout << encoded_query << endl;
		int n = url.length() + 1;
		char* url_encoded = new char[n + strlen(encoded_query)];
		strcpy(url_encoded, url.c_str());
		strcat(url_encoded, encoded_query);
		// cout << url_encoded << endl;
		curl_easy_setopt(_curl, CURLOPT_URL, url_encoded);
		curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, callback_string);
		curl_easy_setopt(_curl, CURLOPT_WRITEDATA, http_response.get());
		_res = curl_easy_perform(_curl);

		long http_code = 0;
		curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &http_code);
		curl_easy_cleanup(_curl);
		delete[] url_encoded;

		if(http_code == 200)
			return *http_response.get();

		return "";
	}

	void ElasticLogger::BulkInsert(const std::string index, std::vector<std::string> datum)
	{
		if(!_ok)
			return;

		std::string test = "";
		for(std::vector<std::string>::iterator it = datum.begin() ; it != datum.end() ; ++it)
		{
			test += "{\"index\":{\"_index\":\"" + index + "\"";
			test += ",\"_type\":\"doc\"";
			test += "}}\n" + (*it) + '\n';
		}
		// curl_easy_setopt(_curl_bulk, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(_curl_bulk, CURLOPT_POSTFIELDS, test.c_str());
		curl_easy_setopt(_curl_bulk, CURLOPT_POSTFIELDSIZE, strlen(test.c_str()));
		curl_easy_setopt(_curl_bulk, CURLOPT_WRITEFUNCTION, callback_dont_care);
		
		_res = curl_easy_perform(_curl_bulk);

		// cout << endl << test << endl;
	}

	void ElasticLogger::Log(const Entry* e)
	{

	}
}
}


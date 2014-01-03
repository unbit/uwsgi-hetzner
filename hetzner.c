#include <uwsgi.h>
#include <curl/curl.h>

extern struct uwsgi_server uwsgi;

struct hetzner_failoverip_config {
        char *username;
        char *password;
        char *url;
        char *timeout;
        char *ip;
        char *active_ip;
        char *ssl_no_verify;
};

#define HETNZER_FAILOVERIP_URL "https://robot-ws.your-server.de/failover/"

static size_t hetzner_curl_writefunc(void *ptr, size_t size, size_t nmemb, void *foo) {
	size_t len = size*nmemb;
	uwsgi_log("%.*s\n", len, (char *)ptr);
	return len;
}

static int action_failoverip(struct uwsgi_legion *ul, char *arg) {
	int ret = -1;
	char *url = NULL;
	char *auth = NULL;
	struct hetzner_failoverip_config hfc;
	memset(&hfc, 0, sizeof(struct hetzner_failoverip_config));

	if (uwsgi_kvlist_parse(arg, strlen(arg), ',', '=',
		"username", &hfc.username,	
		"password", &hfc.password,	
		"user", &hfc.username,	
		"pass", &hfc.password,	
		"url", &hfc.url,	
		"timeout", &hfc.timeout,	
		"ip", &hfc.ip,	
		"active_ip", &hfc.active_ip,	
		"active_server_ip", &hfc.active_ip,	
		"ssl_no_verify", &hfc.ssl_no_verify,	
	NULL)) {
		uwsgi_log("unable to parse hetzner-failopeverip options\n");
		return -1;
	}

	if (!hfc.username || !hfc.password || !hfc.ip || !hfc.active_ip) {
		uwsgi_log("hetzner-failopeverip action requires username, password, ip and active_ip keys\n");
		goto clear;	
	}

	if (hfc.url) {
		url = uwsgi_str(hfc.url);
	}
	else {
		url = uwsgi_concat2(HETNZER_FAILOVERIP_URL, hfc.ip);
	}

	struct curl_httppost *formpost = NULL;
        struct curl_httppost *lastptr=NULL;

	curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "active_server_ip", CURLFORM_COPYCONTENTS, hfc.active_ip, CURLFORM_END);

	CURL *curl = curl_easy_init();
        if (!curl) {
                curl_formfree(formpost);
		goto clear;
        }

	int timeout = 60;
	if (hfc.timeout) timeout = atoi(hfc.timeout);

	curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timeout);
        curl_easy_setopt(curl, CURLOPT_URL, url);

	curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
	auth = uwsgi_concat3(hfc.username, ":", hfc.password);

	curl_easy_setopt(curl, CURLOPT_USERPWD, auth);

        if (hfc.ssl_no_verify) {
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        }

        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, hetzner_curl_writefunc);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
                uwsgi_log("[hetzner-failoverip] curl error: %s\n", curl_easy_strerror(res));
                curl_formfree(formpost);
                curl_easy_cleanup(curl);
		goto clear;
        }

	long http_code = 0;
#ifdef CURLINFO_RESPONSE_CODE
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
#else
	curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &http_code);
#endif
	// allows Ok and Conflict
	uwsgi_log("[hetzner-failoverip] HTTP status code: %ld\n", http_code);
	if (http_code == 200 || http_code == 409) {
		ret = 0;
	}

        curl_formfree(formpost);
        curl_easy_cleanup(curl);

clear:
	if (url) free(url);
	if (auth) free(auth);
	if (hfc.username) free(hfc.username);
	if (hfc.password) free(hfc.password);
	if (hfc.url) free(hfc.url);
	if (hfc.timeout) free(hfc.timeout);
	if (hfc.ip) free(hfc.ip);
	if (hfc.active_ip) free(hfc.active_ip);
	if (hfc.ssl_no_verify) free(hfc.ssl_no_verify);
	return ret;	
}

static void hetzner_register() {
	struct uwsgi_legion_action *ula = uwsgi_legion_action_register("hetzner-failoverip", action_failoverip);
	ula->log_msg = "running hetzner's ip failover procedure...";
}

struct uwsgi_plugin hetzner_plugin = {
        .name = "hetzner",
        .on_load = hetzner_register,
};


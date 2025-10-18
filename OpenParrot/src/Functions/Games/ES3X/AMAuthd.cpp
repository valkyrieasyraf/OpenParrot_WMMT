#include <StdInc.h>
#pragma optimize("", off)
#include <iphlpapi.h>
#include <winsock2.h>
#include <sysinfoapi.h>
#include "Utility/InitFunction.h"
#include "Functions/Global.h"
#include <string.h>
#include "Utility/json.hpp"
#include <windows.h>

#ifndef _M_IX86
u_short HttpPort = 80;

using json = nlohmann::json;

static std::string UpdateGameVersion(const std::string& configText, const std::string& gameVersion1, const std::string& gameVersion2)
{
	std::string result = configText;

	std::string keyCacfg = "cacfg-game_ver=";
	size_t pos = result.find(keyCacfg);
	if (pos != std::string::npos) {
		size_t end = result.find("\n", pos);
		if (end == std::string::npos) end = result.size();
		result.replace(pos + keyCacfg.size(), end - pos - keyCacfg.size(), gameVersion2);
	}

	std::string keyAmucfg = "amucfg-game_rev=";
	pos = result.find(keyAmucfg);
	if (pos != std::string::npos) {
		size_t end = result.find("\n", pos);
		if (end == std::string::npos) end = result.size();
		result.replace(pos + keyAmucfg.size(), end - pos - keyAmucfg.size(), gameVersion1);
	}

	return result;
}

static std::pair<std::string, std::string> FormatSoftwareRevisionDual(int softwareRevision)
{
	char buf[6];
	snprintf(buf, sizeof(buf), "%05d", softwareRevision);
	std::string str(buf);
	std::string firstDigit = str.substr(0, 1);
	std::string lastFour = str.substr(1);
	std::string formatted = lastFour.substr(0, 2) + "." + lastFour.substr(2, 2);
	return { firstDigit, formatted };
}

// 读取文件内容到 string
static std::string ReadFileToString(const std::string& path)
{
	FILE* f = fopen(path.c_str(), "rb");
	if (!f) return "";

	fseek(f, 0, SEEK_END);
	size_t size = ftell(f);
	fseek(f, 0, SEEK_SET);

	std::string content;
	content.resize(size);

	fread(&content[0], 1, size, f);
	fclose(f);
	return content;
}

// 根据 gameVer 获取 softwareRevision
static int GetSoftwareRevision(const std::string& exePath, const std::string& gameVer)
{
	size_t lastSlash = exePath.find_last_of("\\/");
	if (lastSlash == std::string::npos) return -1;
	std::string parentDir = exePath.substr(0, lastSlash);
	std::string jsonPath = parentDir + "\\mtx_config.json";

	std::string content = ReadFileToString(jsonPath);
	if (content.empty()) return -1;

	json j;
	try {
		j = json::parse(content);
	}
	catch (...) {
		return -1;
	}

	for (const auto& item : j) {
		if (item.contains("presetGame") && item["presetGame"].is_string()) {
			if (item["presetGame"] == gameVer) {
				if (item.contains("softwareRevision")) {
					// 支持数字或字符串
					if (item["softwareRevision"].is_number_integer()) {
						return item["softwareRevision"];
					}
					else if (item["softwareRevision"].is_string()) {
						try {
							return std::stoi(item["softwareRevision"].get<std::string>());
						}
						catch (...) {
							return -1;
						}
					}
				}
			}
		}
	}

	return -1;
}

// PrepareAMAuth
static const unsigned char AMConfigWM5[] = R"(
[AMUpdaterConfig]
amucfg-title=WANGAN MIDNIGHT MAXIMUM TUNE 5
amucfg-lang=JP
amucfg-countdown=5
amucfg-h_resol=1360
amucfg-v_resol=768
amucfg-logfile=.\amupdater.log
amucfg-game_rev=1

[AMAuthdConfig]
amdcfg-authType=ALL.NET
amdcfg-sleepTime=50
amdcfg-resoNameTimeout=180
amdcfg-writableConfig=.\WritableConfig.ini
amdcfg-showConsole=ENABLE
amdcfg-logfile=
amdcfg-export_log=
amdcfg-offlineMode=DISABLE

[AllnetConfig]
allcfg-gameID=SBWJ
allcfg-gameVer=5.00

[AllnetOptionRevalTime]
allopt-reval_hour=7
allopt-reval_minute=0
allopt-reval_second=0

[AllnetOptionTimeout]
allopt-timeout_connect=60000
allopt-timeout_send=60000
allopt-timeout_recv=60000

[MuchaAppConfig]
appcfg-logfile=.\muchaapp.log
appcfg-loglevel=INFO

[MuchaSysConfig]
syscfg-daemon_exe=.\MuchaBin\muchacd.exe
syscfg-daemon_pidfile=.\MuchaBin\muchacd.pid
syscfg-daemon_logfile=.\MuchaBin\muchacd.log
syscfg-daemon_loglevel=INFO
syscfg-daemon_listen=tcp:0.0.0.0:12345
syscfg-client_connect=tcp:127.0.0.1:12345

[MuchaCAConfig]
cacfg-game_cd=WM51
cacfg-game_ver=00.05
cacfg-game_board_type=0
cacfg-game_board_id=WM5
cacfg-auth_server_url=https://0.0.0.0:10082/
cacfg-auth_server_sslverify=0
cacfg-auth_server_sslcafile=.\front.mucha-prd.nbgi-amnet.jp.cacert.pem
cacfg-auth_server_timeout=600
cacfg-interval_ainfo_renew=10
cacfg-interval_ainfo_retry=10

[MuchaDtConfig]
dtcfg-dl_product_id=0x57355031
dtcfg-dl_chunk_size=0x10000
dtcfg-dl_image_path=.\dl_image
dtcfg-dl_image_size=0
dtcfg-dl_image_type=RAW
dtcfg-dl_image_crypt_key=0x45520913
dtcfg-dl_log_level=INFO
dtcfg-dl_lan_crypt_key=0xfz26s7201m68952x
dtcfg-dl_lan_broadcast_interval=1000
dtcfg-dl_lan_udp_port=8765
dtcfg-dl_lan_bandwidth_limit=0
dtcfg-dl_lan_broadcast_address=0.0.0.0
dtcfg-dl_wan_retry_limit=
dtcfg-dl_wan_retry_interval=
dtcfg-dl_wan_send_timeout=
dtcfg-dl_wan_recv_timeout=
dtcfg-dl_lan_retry_limit=
dtcfg-dl_lan_retry_interval=
dtcfg-dl_lan_send_timeout=
dtcfg-dl_lan_recv_timeout=

[MuchaDtModeConfig]
dtmode-io_dir=E:\
dtmode-io_file=WM510JPN
dtmode-io_conv=DECEXP
dtmode-io_passphrase=Qx8hJ1KilweAp5Xm
)";

static const unsigned char AMConfigW5X[] = R"(
[AMUpdaterConfig]
amucfg-title=WANGAN MIDNIGHT MAXIMUM TUNE 5DX
amucfg-lang=JP
amucfg-countdown=5
amucfg-h_resol=1360
amucfg-v_resol=768
amucfg-logfile=.\amupdater.log
amucfg-game_rev=2

[AMAuthdConfig]
amdcfg-authType=ALL.NET
amdcfg-sleepTime=50
amdcfg-resoNameTimeout=180
amdcfg-writableConfig=.\WritableConfig.ini
amdcfg-showConsole=ENABLE
amdcfg-logfile=
amdcfg-export_log=
amdcfg-offlineMode=DISABLE

[AllnetConfig]
allcfg-gameID=SBWJ
allcfg-gameVer=5.00

[AllnetOptionRevalTime]
allopt-reval_hour=7
allopt-reval_minute=0
allopt-reval_second=0

[AllnetOptionTimeout]
allopt-timeout_connect=60000
allopt-timeout_send=60000
allopt-timeout_recv=60000

[MuchaAppConfig]
appcfg-logfile=.\muchaapp.log
appcfg-loglevel=INFO

[MuchaSysConfig]
syscfg-daemon_exe=.\MuchaBin\muchacd.exe
syscfg-daemon_pidfile=.\MuchaBin\muchacd.pid
syscfg-daemon_logfile=.\MuchaBin\muchacd.log
syscfg-daemon_loglevel=INFO
syscfg-daemon_listen=tcp:0.0.0.0:12345
syscfg-client_connect=tcp:127.0.0.1:12345

[MuchaCAConfig]
cacfg-game_cd=W5X2
cacfg-game_ver=00.02
cacfg-game_board_type=0
cacfg-game_board_id=W5X
cacfg-auth_server_url=https://0.0.0.0:10082/
cacfg-auth_server_sslverify=0
cacfg-auth_server_sslcafile=.\front.mucha-prd.nbgi-amnet.jp.cacert.pem
cacfg-auth_server_timeout=600
cacfg-interval_ainfo_renew=10
cacfg-interval_ainfo_retry=10

[MuchaDtConfig]
dtcfg-dl_product_id=0x57355831
dtcfg-dl_chunk_size=0x10000
dtcfg-dl_image_path=.\dl_image
dtcfg-dl_image_size=0
dtcfg-dl_image_type=RAW
dtcfg-dl_image_crypt_key=0x45520913
dtcfg-dl_log_level=INFO
dtcfg-dl_lan_crypt_key=0xfz26s7201m68952x
dtcfg-dl_lan_broadcast_interval=1000
dtcfg-dl_lan_udp_port=8765
dtcfg-dl_lan_bandwidth_limit=0
dtcfg-dl_lan_broadcast_address=0.0.0.0
dtcfg-dl_wan_retry_limit=
dtcfg-dl_wan_retry_interval=
dtcfg-dl_wan_send_timeout=
dtcfg-dl_wan_recv_timeout=
dtcfg-dl_lan_retry_limit=
dtcfg-dl_lan_retry_interval=
dtcfg-dl_lan_send_timeout=
dtcfg-dl_lan_recv_timeout=

[MuchaDtModeConfig]
dtmode-io_dir=E:\
dtmode-io_file=W5X10JPN
dtmode-io_conv=DECEXP
dtmode-io_passphrase=Qx8hJ1KilweAp5Xm
)";

static const unsigned char AMConfigW5P[] = R"(
[AMUpdaterConfig]
amucfg-title=WANGAN MIDNIGHT MAXIMUM TUNE 5DX PLUS
amucfg-lang=JP
amucfg-countdown=5
amucfg-h_resol=1360
amucfg-v_resol=768
amucfg-logfile=.\amupdater.log
amucfg-game_rev=3

[AMAuthdConfig]
amdcfg-authType=ALL.NET
amdcfg-sleepTime=50
amdcfg-resoNameTimeout=180
amdcfg-writableConfig=.\WritableConfig.ini
amdcfg-showConsole=ENABLE
amdcfg-logfile=
amdcfg-export_log=
amdcfg-offlineMode=DISABLE

[AllnetConfig]
allcfg-gameID=SBWJ
allcfg-gameVer=7.00

[AllnetOptionRevalTime]
allopt-reval_hour=7
allopt-reval_minute=0
allopt-reval_second=0

[AllnetOptionTimeout]
allopt-timeout_connect=60000
allopt-timeout_send=60000
allopt-timeout_recv=60000

[MuchaAppConfig]
appcfg-logfile=.\muchaapp.log
appcfg-loglevel=INFO

[MuchaSysConfig]
syscfg-daemon_exe=.\MuchaBin\muchacd.exe
syscfg-daemon_pidfile=.\MuchaBin\muchacd.pid
syscfg-daemon_logfile=.\MuchaBin\muchacd.log
syscfg-daemon_loglevel=INFO
syscfg-daemon_listen=tcp:0.0.0.0:12345
syscfg-client_connect=tcp:127.0.0.1:12345

[MuchaCAConfig]
cacfg-game_cd=W5P1
cacfg-game_ver=05.00
cacfg-game_board_type=0
cacfg-game_board_id=W5P
cacfg-auth_server_url=https://0.0.0.0:10082/
cacfg-auth_server_sslverify=0
cacfg-auth_server_sslcafile=.\front.mucha-prd.nbgi-amnet.jp.cacert.pem
cacfg-auth_server_timeout=600
cacfg-interval_ainfo_renew=10
cacfg-interval_ainfo_retry=10

[MuchaDtConfig]
dtcfg-dl_product_id=0x57355031
dtcfg-dl_chunk_size=0x10000
dtcfg-dl_image_path=.\dl_image
dtcfg-dl_image_size=0
dtcfg-dl_image_type=RAW
dtcfg-dl_image_crypt_key=0x19535520
dtcfg-dl_log_level=INFO
dtcfg-dl_lan_crypt_key=0x75lx0126s476ss3b
dtcfg-dl_lan_broadcast_interval=1000
dtcfg-dl_lan_udp_port=8765
dtcfg-dl_lan_bandwidth_limit=0
dtcfg-dl_lan_broadcast_address=0.0.0.0
dtcfg-dl_wan_retry_limit=
dtcfg-dl_wan_retry_interval=
dtcfg-dl_wan_send_timeout=
dtcfg-dl_wan_recv_timeout=
dtcfg-dl_lan_retry_limit=
dtcfg-dl_lan_retry_interval=
dtcfg-dl_lan_send_timeout=
dtcfg-dl_lan_recv_timeout=

[MuchaDtModeConfig]
dtmode-io_dir=E:\
dtmode-io_file=W5P10JPN
dtmode-io_conv=DECEXP
dtmode-io_passphrase=Qx8hJ1KilweAp5Xm
)";

static const unsigned char AMConfigWM6[] = R"(
[AMUpdaterConfig]
;; AMUpdater Configuration
amucfg-title=WANGAN MIDNIGHT MAXIMUM TUNE 6
amucfg-lang=EN
amucfg-countdown=5
amucfg-h_resol=1360
amucfg-v_resol=768
amucfg-logfile=.\amupdater.log
amucfg-game_rev=1

[AMAuthdConfig]
amdcfg-authType=ALL.NET
amdcfg-sleepTime=50
amdcfg-resoNameTimeout=180
amdcfg-writableConfig=.\WritableConfig.ini
amdcfg-showConsole=ENABLE
amdcfg-logfile=
amdcfg-export_log=
amdcfg-offlineMode=DISABLE

[AllnetConfig]
allcfg-gameID=SBWJ
allcfg-gameVer=9.00

[AllnetOptionRevalTime]
allopt-reval_hour=7
allopt-reval_minute=0
allopt-reval_second=0

[AllnetOptionTimeout]
allopt-timeout_connect=60000
allopt-timeout_send=60000
allopt-timeout_recv=60000

[MuchaAppConfig]
appcfg-logfile=.\muchaapp.log
appcfg-loglevel=INFO

[MuchaSysConfig]
syscfg-daemon_exe=.\MuchaBin\muchacd.exe
syscfg-daemon_pidfile=.\muchacd.pid
syscfg-daemon_logfile=.\muchacd.log
syscfg-daemon_loglevel=INFO
syscfg-daemon_listen=tcp:0.0.0.0:12345
syscfg-client_connect=tcp:127.0.0.1:12345

[MuchaCAConfig]
cacfg-game_cd=WM61
cacfg-game_ver=03.04
cacfg-game_board_type=0
cacfg-game_board_id=WM6
cacfg-auth_server_url=https://0.0.0.0:10082/
cacfg-auth_server_sslverify=0
cacfg-auth_server_sslcafile=.\front.mucha-prd.nbgi-amnet.jp.cacert.pem
cacfg-auth_server_timeout=300
cacfg-interval_ainfo_renew=1800
cacfg-interval_ainfo_retry=60

[MuchaDtConfig]
dtcfg-dl_product_id=0x574d3631
dtcfg-dl_chunk_size=0x10000
dtcfg-dl_image_path=.\dl_image
dtcfg-dl_image_size=0
dtcfg-dl_image_type=RAW
dtcfg-dl_image_crypt_key=0xdd149aba
dtcfg-dl_log_level=INFO
dtcfg-dl_lan_crypt_key=0xb94ba9fd15258bcb
dtcfg-dl_lan_broadcast_interval=1000
dtcfg-dl_lan_udp_port=8765
dtcfg-dl_lan_bandwidth_limit=0
dtcfg-dl_lan_broadcast_address=0.0.0.0
dtcfg-dl_wan_retry_limit=
dtcfg-dl_wan_retry_interval=
dtcfg-dl_wan_send_timeout=
dtcfg-dl_wan_recv_timeout=
dtcfg-dl_lan_retry_limit=
dtcfg-dl_lan_retry_interval=
dtcfg-dl_lan_send_timeout=
dtcfg-dl_lan_recv_timeout=

[MuchaDtModeConfig]
dtmode-io_dir=E:\
dtmode-io_file=WM610JPN
dtmode-io_conv=DECEXP
dtmode-io_passphrase=Qx8hJ1KilweAp5Xm
)";

static const unsigned char AMConfigW6R[] = R"(
[AMUpdaterConfig]
;; AMUpdater Configuration
amucfg-title=WANGAN MIDNIGHT MAXIMUM TUNE 6R
amucfg-lang=EN
amucfg-countdown=5
amucfg-h_resol=1360
amucfg-v_resol=768
amucfg-logfile=.\amupdater.log
amucfg-game_rev=2

[AMAuthdConfig]
amdcfg-authType=ALL.NET
amdcfg-sleepTime=50
amdcfg-resoNameTimeout=180
amdcfg-writableConfig=.\WritableConfig.ini
amdcfg-showConsole=ENABLE
amdcfg-logfile=
amdcfg-export_log=
amdcfg-offlineMode=DISABLE

[AllnetConfig]
allcfg-gameID=SBWJ
allcfg-gameVer=5.00

[AllnetOptionRevalTime]
allopt-reval_hour=7
allopt-reval_minute=0
allopt-reval_second=0

[AllnetOptionTimeout]
allopt-timeout_connect=60000
allopt-timeout_send=60000
allopt-timeout_recv=60000

[MuchaAppConfig]
appcfg-logfile=.\muchaapp.log
appcfg-loglevel=INFO

[MuchaSysConfig]
syscfg-daemon_exe=.\MuchaBin\muchacd.exe
syscfg-daemon_pidfile=.\muchacd.pid
syscfg-daemon_logfile=.\muchacd.log
syscfg-daemon_loglevel=INFO
syscfg-daemon_listen=tcp:0.0.0.0:12345
syscfg-client_connect=tcp:127.0.0.1:12345

[MuchaCAConfig]
cacfg-game_cd=W6R1
cacfg-game_ver=00.08
cacfg-game_board_type=0
cacfg-game_board_id=W6R
cacfg-auth_server_url=https://0.0.0.0:10082/
cacfg-auth_server_sslverify=0
cacfg-auth_server_sslcafile=.\front.mucha-prd.nbgi-amnet.jp.cacert.pem
cacfg-auth_server_timeout=300
cacfg-interval_ainfo_renew=1800
cacfg-interval_ainfo_retry=60

[MuchaDtConfig]
dtcfg-dl_product_id=0x57355031
dtcfg-dl_chunk_size=0x10000
dtcfg-dl_image_path=.\dl_image
dtcfg-dl_image_size=0
dtcfg-dl_image_type=RAW
dtcfg-dl_image_crypt_key=0x45520913
dtcfg-dl_log_level=INFO
dtcfg-dl_lan_crypt_key=0xfz26s7201m68952x
dtcfg-dl_lan_broadcast_interval=1000
dtcfg-dl_lan_udp_port=8765
dtcfg-dl_lan_bandwidth_limit=0
dtcfg-dl_lan_broadcast_address=0.0.0.0
dtcfg-dl_wan_retry_limit=
dtcfg-dl_wan_retry_interval=
dtcfg-dl_wan_send_timeout=
dtcfg-dl_wan_recv_timeout=
dtcfg-dl_lan_retry_limit=
dtcfg-dl_lan_retry_interval=
dtcfg-dl_lan_send_timeout=
dtcfg-dl_lan_recv_timeout=

[MuchaDtModeConfig]
dtmode-io_dir=E:\
dtmode-io_file=W6R10JPN
dtmode-io_conv=DECEXP
dtmode-io_passphrase=Qx8hJ1KilweAp5Xm
)";

static const unsigned char AMConfigW6W[] = R"(
[AMUpdaterConfig]
amucfg-title=WANGAN MIDNIGHT MAXIMUM TUNE 6RR
amucfg-lang=EN
amucfg-countdown=5
amucfg-h_resol=1360
amucfg-v_resol=768
amucfg-logfile=.\amupdater.log
amucfg-game_rev=3

[AMAuthdConfig]
amdcfg-authType=ALL.NET
amdcfg-sleepTime=50
amdcfg-resoNameTimeout=180
amdcfg-writableConfig=.\WritableConfig.ini
amdcfg-showConsole=ENABLE
amdcfg-logfile=
amdcfg-export_log=
amdcfg-offlineMode=DISABLE

[AllnetConfig]
allcfg-gameID=SBWJ
allcfg-gameVer=5.00

[AllnetOptionRevalTime]
allopt-reval_hour=7
allopt-reval_minute=0
allopt-reval_second=0

[AllnetOptionTimeout]
allopt-timeout_connect=60000
allopt-timeout_send=60000
allopt-timeout_recv=60000

[MuchaAppConfig]
appcfg-logfile=.\muchaapp.log
appcfg-loglevel=INFO

[MuchaSysConfig]
syscfg-daemon_exe=.\MuchaBin\muchacd.exe
syscfg-daemon_pidfile=.\muchacd.pid
syscfg-daemon_logfile=.\muchacd.log
syscfg-daemon_loglevel=INFO
syscfg-daemon_listen=tcp:0.0.0.0:12345
syscfg-client_connect=tcp:127.0.0.1:12345

[MuchaCAConfig]
cacfg-game_cd=W6W1
cacfg-game_ver=05.03
cacfg-game_board_type=0
cacfg-game_board_id=W6W
cacfg-auth_server_url=https://0.0.0.0:10082/
cacfg-auth_server_sslverify=0
cacfg-auth_server_sslcafile=.\front.mucha-prd.nbgi-amnet.jp.cacert.pem
cacfg-auth_server_timeout=300
cacfg-interval_ainfo_renew=1800
cacfg-interval_ainfo_retry=60

[MuchaDtConfig]
dtcfg-dl_product_id=0x57355031
dtcfg-dl_chunk_size=0x10000
dtcfg-dl_image_path=.\dl_image
dtcfg-dl_image_size=0
dtcfg-dl_image_type=RAW
dtcfg-dl_image_crypt_key=0x45520913
dtcfg-dl_log_level=INFO
dtcfg-dl_lan_crypt_key=0xfz26s7201m68952x
dtcfg-dl_lan_broadcast_interval=1000
dtcfg-dl_lan_udp_port=8765
dtcfg-dl_lan_bandwidth_limit=0
dtcfg-dl_lan_broadcast_address=0.0.0.0
dtcfg-dl_wan_retry_limit=
dtcfg-dl_wan_retry_interval=
dtcfg-dl_wan_send_timeout=
dtcfg-dl_wan_recv_timeout=
dtcfg-dl_lan_retry_limit=
dtcfg-dl_lan_retry_interval=
dtcfg-dl_lan_send_timeout=
dtcfg-dl_lan_recv_timeout=

[MuchaDtModeConfig]
dtmode-io_dir=E:\
dtmode-io_file=W6W10JPN
dtmode-io_conv=DECEXP
dtmode-io_passphrase=Qx8hJ1KilweAp5Xm
)";

static const unsigned char AMConfigW6P[] = R"(
[AMUpdaterConfig]
amucfg-title=WANGAN MIDNIGHT MAXIMUM TUNE 6RR PLUS
amucfg-lang=EN
amucfg-countdown=5
amucfg-h_resol=1360
amucfg-v_resol=768
amucfg-logfile=.\amupdater.log
amucfg-game_rev=3

[AMAuthdConfig]
amdcfg-authType=ALL.NET
amdcfg-sleepTime=50
amdcfg-resoNameTimeout=180
amdcfg-writableConfig=.\WritableConfig.ini
amdcfg-showConsole=ENABLE
amdcfg-logfile=
amdcfg-export_log=
amdcfg-offlineMode=DISABLE

[AllnetConfig]
allcfg-gameID=SBWJ
allcfg-gameVer=7.00

[AllnetOptionRevalTime]
allopt-reval_hour=7
allopt-reval_minute=0
allopt-reval_second=0

[AllnetOptionTimeout]
allopt-timeout_connect=60000
allopt-timeout_send=60000
allopt-timeout_recv=60000

[MuchaAppConfig]
appcfg-logfile=.\muchaapp.log
appcfg-loglevel=INFO

[MuchaSysConfig]
syscfg-daemon_exe=.\MuchaBin\muchacd.exe
syscfg-daemon_pidfile=.\muchacd.pid
syscfg-daemon_logfile=.\muchacd.log
syscfg-daemon_loglevel=INFO
syscfg-daemon_listen=tcp:0.0.0.0:12345
syscfg-client_connect=tcp:127.0.0.1:12345

[MuchaCAConfig]
cacfg-game_cd=W6P1
cacfg-game_ver=00.12
cacfg-game_board_type=0
cacfg-game_board_id=W6P
cacfg-auth_server_url=https://0.0.0.0:10082/
cacfg-auth_server_sslverify=0
cacfg-auth_server_sslcafile=.\front.mucha-prd.nbgi-amnet.jp.cacert.pem
cacfg-auth_server_timeout=300
cacfg-interval_ainfo_renew=1800
cacfg-interval_ainfo_retry=60

[MuchaDtConfig]
dtcfg-dl_product_id=0x57355031
dtcfg-dl_chunk_size=0x10000
dtcfg-dl_image_path=.\dl_image
dtcfg-dl_image_size=0
dtcfg-dl_image_type=RAW
dtcfg-dl_image_crypt_key=0x45520913
dtcfg-dl_log_level=INFO
dtcfg-dl_lan_crypt_key=0xfz26s7201m68952x
dtcfg-dl_lan_broadcast_interval=1000
dtcfg-dl_lan_udp_port=8765
dtcfg-dl_lan_bandwidth_limit=0
dtcfg-dl_lan_broadcast_address=0.0.0.0
dtcfg-dl_wan_retry_limit=
dtcfg-dl_wan_retry_interval=
dtcfg-dl_wan_send_timeout=
dtcfg-dl_wan_recv_timeout=
dtcfg-dl_lan_retry_limit=
dtcfg-dl_lan_retry_interval=
dtcfg-dl_lan_send_timeout=
dtcfg-dl_lan_recv_timeout=

[MuchaDtModeConfig]
dtmode-io_dir=E:\
dtmode-io_file=W6P10JPN
dtmode-io_conv=DECEXP
dtmode-io_passphrase=Qx8hJ1KilweAp5Xm
)";

static void PrepareAMAuth() {
	// Prepare For AMAuth
	FILE* AMConfigWrite = fopen("AMConfig.ini", "w");
	std::string GameVersion = config["Authentication"]["GameVersion"];
	std::string tpuiLocation = config["TeknoParrotUi"]["exeLocation"];

	std::string gameVer = "W6W";
	std::string gameSetVer = GameVersion;
	if (gameSetVer == "WM510JPN05") gameVer = "WM5";
	else if (gameSetVer == "W5X10JPN02") gameVer = "W5X";
	else if (gameSetVer == "W5P10JPN05") gameVer = "W5P";
	else if (gameSetVer == "WM610JPN04") gameVer = "WM6";
	else if (gameSetVer == "W6R10JPN08") gameVer = "W6R";
	else if (gameSetVer == "W6W10JPN05") gameVer = "W6W";
	else if (gameSetVer == "W6P10JPN00") gameVer = "W6P";
	else gameVer = "W6W";

	// Wangan Midnight Maximum Tune 5
	if (gameVer == "WM5")
	{
		int revision = GetSoftwareRevision(tpuiLocation, gameVer);
		std::string finalConfig;
		if (revision != -1) {
			auto [gameVersion1, gameVersion2] = FormatSoftwareRevisionDual(revision);

			std::string configText(reinterpret_cast<const char*>(AMConfigWM5), sizeof(AMConfigWM5));
			finalConfig = UpdateGameVersion(configText, gameVersion1, gameVersion2);
		}
		else {
			finalConfig = std::string(reinterpret_cast<const char*>(AMConfigWM5), sizeof(AMConfigWM5));
		}
		fwrite(finalConfig.c_str(), 1, finalConfig.size(), AMConfigWrite);
		fclose(AMConfigWrite);
	}
	// Wangan Midnight Maximum Tune 5DX
	else if (gameVer == "W5X")
	{
		int revision = GetSoftwareRevision(tpuiLocation, gameVer);
		std::string finalConfig;
		if (revision != -1) {
			auto [gameVersion1, gameVersion2] = FormatSoftwareRevisionDual(revision);

			std::string configText(reinterpret_cast<const char*>(AMConfigW5X), sizeof(AMConfigW5X));
			finalConfig = UpdateGameVersion(configText, gameVersion1, gameVersion2);
		}
		else {
			finalConfig = std::string(reinterpret_cast<const char*>(AMConfigW5X), sizeof(AMConfigW5X));
		}
		fwrite(finalConfig.c_str(), 1, finalConfig.size(), AMConfigWrite);
		fclose(AMConfigWrite);
	}
	// Wangan Midnight Maximum Tune 5DX+
	else if (gameVer == "W5P")
	{
		int revision = GetSoftwareRevision(tpuiLocation, gameVer);
		std::string finalConfig;
		if (revision != -1) {
			auto [gameVersion1, gameVersion2] = FormatSoftwareRevisionDual(revision);

			std::string configText(reinterpret_cast<const char*>(AMConfigW5P), sizeof(AMConfigW5P));
			finalConfig = UpdateGameVersion(configText, gameVersion1, gameVersion2);
		}
		else {
			finalConfig = std::string(reinterpret_cast<const char*>(AMConfigW5P), sizeof(AMConfigW5P));
		}
		fwrite(finalConfig.c_str(), 1, finalConfig.size(), AMConfigWrite);
		fclose(AMConfigWrite);
	}
	// Wangan Midnight Maximum Tune 6
	else if (gameVer == "WM6")
	{
		int revision = GetSoftwareRevision(tpuiLocation, gameVer);
		std::string finalConfig;
		if (revision != -1) {
			auto [gameVersion1, gameVersion2] = FormatSoftwareRevisionDual(revision);

			std::string configText(reinterpret_cast<const char*>(AMConfigWM6), sizeof(AMConfigWM6));
			finalConfig = UpdateGameVersion(configText, gameVersion1, gameVersion2);
		}
		else {
			finalConfig = std::string(reinterpret_cast<const char*>(AMConfigWM6), sizeof(AMConfigWM6));
		}
		fwrite(finalConfig.c_str(), 1, finalConfig.size(), AMConfigWrite);
		fclose(AMConfigWrite);
	}
	// Wangan Midnight Maximum Tune 6R
	else if (gameVer == "W6R")
	{
		int revision = GetSoftwareRevision(tpuiLocation, gameVer);
		std::string finalConfig;
		if (revision != -1) {
			auto [gameVersion1, gameVersion2] = FormatSoftwareRevisionDual(revision);

			std::string configText(reinterpret_cast<const char*>(AMConfigW6R), sizeof(AMConfigW6R));
			finalConfig = UpdateGameVersion(configText, gameVersion1, gameVersion2);
		}
		else {
			finalConfig = std::string(reinterpret_cast<const char*>(AMConfigW6R), sizeof(AMConfigW6R));
		}
		fwrite(finalConfig.c_str(), 1, finalConfig.size(), AMConfigWrite);
		fclose(AMConfigWrite);
	}
	// Wangan Midnight Maximum Tune 6RR
	else if (gameVer == "W6W")
	{
		int revision = GetSoftwareRevision(tpuiLocation, gameVer);
		std::string finalConfig;
		if (revision != -1) {
			auto [gameVersion1, gameVersion2] = FormatSoftwareRevisionDual(revision);

			std::string configText(reinterpret_cast<const char*>(AMConfigW6W), sizeof(AMConfigW6W));
			finalConfig = UpdateGameVersion(configText, gameVersion1, gameVersion2);
		}
		else {
			finalConfig = std::string(reinterpret_cast<const char*>(AMConfigW6W), sizeof(AMConfigW6W));
		}
		fwrite(finalConfig.c_str(), 1, finalConfig.size(), AMConfigWrite);
		fclose(AMConfigWrite);
	}
	// Wangan Midnight Maximum Tune 6RR+
	else if (gameVer == "W6P")
	{
		int revision = GetSoftwareRevision(tpuiLocation, gameVer);
		std::string finalConfig;
		if (revision != -1) {
			auto [gameVersion1, gameVersion2] = FormatSoftwareRevisionDual(revision);

			std::string configText(reinterpret_cast<const char*>(AMConfigW6P), sizeof(AMConfigW6P));
			finalConfig = UpdateGameVersion(configText, gameVersion1, gameVersion2);
		}
		else {
			finalConfig = std::string(reinterpret_cast<const char*>(AMConfigW6P), sizeof(AMConfigW6P));
		}
		fwrite(finalConfig.c_str(), 1, finalConfig.size(), AMConfigWrite);
		fclose(AMConfigWrite);
	}

	char WritableConfigFinal[1024] = "";
	std::string NETID = config["Authentication"]["NetID"];
	strcat(WritableConfigFinal, "[RuntimeConfig]\nmode=\nnetID=");
	strcat(WritableConfigFinal, NETID.c_str());
	strcat(WritableConfigFinal, "\nserialID=");

	FILE* WritableConfigWrite = fopen("WritableConfig.ini", "w");
	fwrite(WritableConfigFinal, 1, strlen(WritableConfigFinal), WritableConfigWrite);
	fclose(WritableConfigWrite);
}

linb::ini myconfig;

static char gatewayAddressStr[256];

bool (WINAPI* orig_SetSystemTime)(const SYSTEMTIME* lpSystemTime);

static bool SetSystemTimeHook(const SYSTEMTIME* lpSystemTime) {
	return 1;
}

u_short(PASCAL FAR* htons_orig)(u_short hostshort);

static u_short htonsHook(u_short hostshort) {
#if _DEBUG
	/*info("htons: %i", hostshort);*/
#endif

	if (hostshort == 80) {
#ifdef _DEBUG
		/*info("replacing port...");*/
#endif
		return htons_orig(HttpPort);
	}
	else {
		return htons_orig(hostshort);
	}
}

int(WSAAPI* g_origgetaddrinfoo)(PCSTR pNodeName, PCSTR pServiceName, const ADDRINFOA* pHints, PADDRINFOA* ppResult);

int WSAAPI getaddrinfoHookAMAuth(PCSTR pNodeName, PCSTR pServiceName, const ADDRINFOA* pHints, PADDRINFOA* ppResult) {
#if _DEBUG
	info("getaddrinfo: %s, %s", pNodeName, pServiceName);
#endif
	if (strcmp(pNodeName, "tenporouter.loc") == 0) {
		return g_origgetaddrinfoo(config["General"]["NetworkAdapterIP"].c_str(), pServiceName, pHints, ppResult);
	}
	else if (strcmp(pNodeName, "bbrouter.loc") == 0) {
		return g_origgetaddrinfoo(config["General"]["NetworkAdapterIP"].c_str(), pServiceName, pHints, ppResult);
	}
	else if (strcmp(pNodeName, "naominet.jp") == 0) {
		return g_origgetaddrinfoo(config["General"]["ServerHost"].c_str(), pServiceName, pHints, ppResult);
	}
	else {
		return g_origgetaddrinfoo(pNodeName, pServiceName, pHints, ppResult);
	}
}

static int WINAPI GetRTTAndHopCountStubAM(_In_ uint32_t DestIpAddress, _Out_ PULONG HopCount, _In_ ULONG MaxHops, _Out_ PULONG RTT) {
	return 1;
}

typedef HRESULT(__stdcall* DllRegisterServerFunc)();

static void dllreg() {
	//iauthdll.dll
	HMODULE hModule = LoadLibrary(L"iauthdll.dll");
	DllRegisterServerFunc DllRegisterServer = (DllRegisterServerFunc)GetProcAddress(hModule, "DllRegisterServer");
	HRESULT hr = DllRegisterServer();
	if (SUCCEEDED(hr)) {
#ifdef DEBUG
		īnfo(true, "iauthdll.dll registered successfully!");
#endif
	}
	else {
		int msgboxID = MessageBox(
			NULL,
			(LPCWSTR)L"iauthdll.dll register failure.",
			(LPCWSTR)L"ALL.Net Online Auth Service Error",
			MB_ICONWARNING | MB_OK
		);
		// There was an error
	}
}

static InitFunction HookAmAuthD64([]() {
	// write config files for mt6
	PrepareAMAuth();
	dllreg();
	MH_Initialize();
	MH_CreateHookApi(L"kernel32.dll", "SetSystemTime", SetSystemTimeHook, (void**)&orig_SetSystemTime);
	MH_CreateHookApi(L"ws2_32.dll", "getaddrinfo", getaddrinfoHookAMAuth, (void**)&g_origgetaddrinfoo);
	MH_CreateHookApi(L"ws2_32.dll", "htons", htonsHook, (void**)&htons_orig);
	MH_CreateHookApi(L"iphlpapi.dll", "GetRTTAndHopCount", GetRTTAndHopCountStubAM, NULL);
	MH_EnableHook(MH_ALL_HOOKS);
	}, GameID::AMAuthd);
#pragma optimize("", on)
#endif
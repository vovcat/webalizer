// make CFLAGS="-O0 -g" all install DESTDIR=$PWD/debian/webalizer
// gcc -g -O0 agent_mangle.c hashtab.o linklist.o preserve.o parser.o output.o dns_resolv.o graphs.o gdfontg.o gdfontl.o gdfontmb.o gdfonts.o gdfontt.o xcode.o -lGeoIP -ldb -lgd -lpng -lz -lm && ./a.out 3 '"Mozilla/5.0 (compatible; Refer.Ru; +http://www.refer.ru)"' '"Mozilla/5.0 (compatible; Dataprovider.com)"' '"GuzzleHttp/6.2.1 curl/7.52.1 PHP/7.2.33-1+0~20200807.47+debian9~1.gbpcb3068"' '"Mozilla/4.0 (compatible; fluid/0.0; +http://www.leak.info/bot.html)"' '"Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.1; Trident/6.0)"' '"Mozilla/5.0 (Windows NT 10.0; WOW64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/84.0.4147.105 Safari/537.36 OPR/70.0.3728.95"' '"Mozilla/5.0  (Windows NT 10.0; Win64; x64)"' '"Mozilla/5.0 (Linux; Android 6.0.1; Nexus 5X Build/MMB29P) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/125.0.6422.65 Mobile Safari/537.36 (compatible; Googlebot/2.1; +http://www.google.com/bot.html)"' '"Mozilla/5.0 (compatible; Linux x86_64; Mail.RU_Bot/Robots/2.0; +https://help.mail.ru/webmaster/indexing/robots)"' '"Mozilla/5.0 (compatible; AwarioBot/1.0; +https://awario.com/bots.html)"' '"Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10_1) AppleWebKit/600.2.5 (KHTML, like Gecko) Version/8.0.2 Safari/600.2.5 (Amazonbot/0.1; +https://developer.amazon.com/support/amazonbot)"' '"Mozilla/5.0 AppleWebKit/537.36 (KHTML, like Gecko; compatible; GPTBot/1.0; +https://openai.com/gptbot)"' '"KOCMOHABT (https://kozmonavt.su/) Mozilla/5.0 (Web Explorer)"' '"Opera/9.63 (Windows NT 5.1; U; MRA 5.10 (build 5196); ru) Presto/2.1.1"' '"Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.8.1.1) Gecko/20061205 Firefox/2.0.0.1 (Debian-2.0.0.1+dfsg-2)"' '"Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.1.5) Gecko/20091102 Firefox/3.5.5 (.NET CLR 3.5.30729)"' '"Mozilla/5.0 \\(Windows NT 10.0\\; Win64\\; x64\\) AppleWebKit/537.36 \\(KHTML, like Gecko\\) Chrome/100.0.4896.60 Safari/537.36"' '"Apache/2.4.34 (Ubuntu) OpenSSL/1.1.1 (internal dummy connection)"'
// gcc -g -O0 agent_mangle.c hashtab.o linklist.o preserve.o parser.o output.o dns_resolv.o graphs.o gdfontg.o gdfontl.o gdfontmb.o gdfonts.o gdfontt.o xcode.o -lGeoIP -ldb -lgd -lpng -lz -lm && cat agent_mangle.txt |./a.out 3 |dwc agent_mangle.txt -

#define ETCDIR "."
#define USE_GEOIP
#define USE_DNS
#define main main_
#include "webalizer.c"
#undef main

int main(int argc, char *argv[])
{
    if (argc > 1) {
        mangle_agent = atoi(argv[1]);
        //printf("mangle_agent %d\n", mangle_agent);
    }
    if (argc > 2) {
        for (int i = 2; i < argc; i++) {
            strncpy(log_rec.agent, argv[i], sizeof(log_rec.agent)-1);
            agent_mangle(log_rec.agent);
            printf("%s\n\t%s\n", argv[i], log_rec.agent);
        }
    } else {
        char buf[4096] = {};
        while (fgets(buf, sizeof(buf)-1, stdin)) {
            if (buf[0] != '"') continue;
            if (buf[0] && buf[strlen(buf)-1] == '\n') buf[strlen(buf)-1] = '\0';
            strncpy(log_rec.agent, buf, sizeof(log_rec.agent)-1);
            agent_mangle(log_rec.agent);
            printf("%s\n\t%s\n", buf, log_rec.agent);
        }
    }
}

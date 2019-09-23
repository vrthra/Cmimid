#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

enum url_type {
    URL_NORMAL,
    URL_OLD_TFTP,
    URL_PREFIX
};

struct url_info {
    char *scheme;
    char *user;
    char *passwd;
    char *host;
    unsigned int port;
    char *path;			/* Includes query */
    enum url_type type;
};

void parse_url(struct url_info *ui){
    char url[PATH_MAX];
    fgets(url, MAX, stdin);
    char *p = url;
    char *q, *r, *s;

    memset(ui, 0, sizeof *ui);

    q = strstr(p, "://");
    if (!q) {
        q = strstr(p, "::");
        if (q) {
            *q = '\000';
            ui->scheme = "tftp";
            ui->host = p;
            ui->path = q+2;
            ui->type = URL_OLD_TFTP;
            return;
        } else {
            ui->path = p;
            ui->type = URL_PREFIX;
            return;
        }
    }

    ui->type = URL_NORMAL;

    ui->scheme = p;
    *q = '\000';
    p = q+3;

    q = strchr(p, '/');
    if (q) {
        *q = '\000';
        ui->path = q+1;
        q = strchr(q+1, '#');
    if (q)
        *q = '\000';
    } else {
        ui->path = "";
    }

    r = strchr(p, '@');
    if (r) {
        ui->user = p;
        *r = '\000';
        s = strchr(p, ':');
        if (s) {
            *s = '\000';
            ui->passwd = s+1;
        }
        p = r+1;
    }

    ui->host = p;
    r = strchr(p, ':');
    if (r) {
        *r = '\000';
        ui->port = atoi(r+1);
    }
}

char *url_escape_unsafe(const char *input){
    const char *p = input;
    unsigned char c;
    char *out, *q;
    int n = 0;

    while ((c = *p++)) {
        if (c < ' ' || c > '~') {
            n += 3;		/* Need escaping */
        } else {
            n++;
        }
    }

    q = out = malloc(n+1);
    while ((c = *p++)) {
        if (c < ' ' || c > '~') {
            q += snprintf(q, 3, "%02X", c);
        } else {
            *q++ = c;
        }
    }

    *q = '\000';

    return out;
}

static int hexdigit(char c){
    if (c >= '0' && c <= '9')
        return c - '0';
    c |= 0x20;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return -1;
}

void url_unescape(char *buffer){
    const char *p = buffer;
    char *q = buffer;
    unsigned char c;
    int x, y;

    while ((c = *p++)) {
        if (c == '%') {
            x = hexdigit(p[0]);
            if (x >= 0) {
                y = hexdigit(p[1]);
                if (y >= 0) {
                    *q++ = (x << 4) + y;
                    p += 2;
                    continue;
                }
            }
        }
        *q++ = c;
    }
    *q = '\000';
}


int main(int argc, char* argv[]) {
    struct url_info url;
    parse_url(&url);
    return 0;
}

#pragma once

/*
 * Copy this file to src/ikuai_cert.h and replace the placeholder with the
 * PEM certificate presented by your iKuai HTTPS endpoint. The real file is
 * ignored by Git so a router-specific certificate is not published.
 */
static const char ikuai_cert_pem[] =
    "-----BEGIN CERTIFICATE-----\n"
    "REPLACE_WITH_YOUR_IKUAI_CERTIFICATE\n"
    "-----END CERTIFICATE-----\n";

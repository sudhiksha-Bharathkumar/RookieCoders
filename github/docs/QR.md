# TrustSeal QR

The QR should contain only the verification URL.

Example:

```text
https://YOUR-FRONTEND-DOMAIN/verify.html?id=TS-1048
```

For multiple packages, change the ID:

```text
https://YOUR-FRONTEND-DOMAIN/verify.html?id=TS-1049
https://YOUR-FRONTEND-DOMAIN/verify.html?id=TS-1050
```

Do not put:
- customer passwords
- OTPs
- API keys
- Wi-Fi credentials
- permanent device secrets

inside the QR.

The QR identifies the TrustSeal/package. The backend should perform authorization and customer verification separately.

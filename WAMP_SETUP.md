# ThinkFast — WAMP Setup Guide

## Why the "Unexpected token '<'" error happens

When the JS calls `/backend/auth.php` and Apache cannot find the file,
it returns an HTML 404 page.  The JS then tries to `JSON.parse()` that
HTML and immediately crashes with:

    Network error: Unexpected token '<', "<!DOCTYPE "... is not valid JSON

The fix is making sure the `DocumentRoot` points to the **project root**
`ThinkFast3\`, not `ThinkFast3\src\gui\`.  Both the GUI and the backend
must be accessible under the same hostname.

---

## Correct folder structure inside WAMP

```
C:\wamp64\www\
└── ThinkFast3\          <-- DocumentRoot for "thinkfast"
    ├── backend\         <-- /backend/auth.php, room.php, etc.
    ├── data\            <-- users.csv, rooms\  (writable by Apache)
    ├── src\
    │   └── gui\         <-- /src/gui/index.html
    └── thinkfast-vhost.conf
```

---

## Step-by-step setup

### 1. Copy project files

Copy the entire `ThinkFast3` folder into:

    C:\wamp64\www\ThinkFast3\

### 2. Enable the VirtualHost config file

Open this file in a text editor (run as Administrator):

    C:\wamp64\bin\apache\apache2.4.XX\conf\httpd.conf

Find the line (it may be commented out):

    # Include conf/extra/httpd-vhosts.conf

Remove the `#` to uncomment it:

    Include conf/extra/httpd-vhosts.conf

### 3. Add the VirtualHost block

Open:

    C:\wamp64\bin\apache\apache2.4.XX\conf\extra\httpd-vhosts.conf

Paste in the contents of `thinkfast-vhost.conf` (included in this project).
Make sure the **localhost** block is also present — without it, your default
`http://localhost/` will break when named vhosts are active.

### 4. Add hosts entry

Open Notepad **as Administrator**, then open:

    C:\Windows\System32\drivers\etc\hosts

Add this line at the bottom:

    127.0.0.1   thinkfast

Save and close.

### 5. Make data\ writable

Right-click `C:\wamp64\www\ThinkFast3\data\`, Properties → Security.
Give the Apache service account (`IUSR` or `Everyone` for local dev)
**Modify** permission so PHP can read/write `users.csv` and create `rooms\`.

### 6. Restart WAMP

Left-click the WAMP tray icon → **Restart All Services**.
All icons in the tray should turn green.

### 7. Test PHP

Visit:  http://thinkfast/backend/auth.php

You should see JSON like `{"ok":false,"error":"Missing action"}`,
not an HTML page.  If you see HTML, PHP is not running — check that
the PHP module is enabled in WAMP (tray icon → PHP → version).

### 8. Open the game

    http://thinkfast/src/gui/

Or just:  http://thinkfast/   (auto-redirects to /src/gui/)

---

## Multiplayer: connecting from other devices on your network

1. Find your local IP:  open Command Prompt → `ipconfig`
   Look for `IPv4 Address`, e.g. `192.168.1.42`

2. On the other device, add this to **their** hosts file:

       192.168.1.42   thinkfast

3. They can now open:  http://thinkfast/src/gui/

4. Host creates a room → shares the 6-character code → others join.

---

## Troubleshooting

| Symptom | Cause | Fix |
|---|---|---|
| `Unexpected token '<'` | Wrong DocumentRoot | Set DocumentRoot to `ThinkFast3\`, not `ThinkFast3\src\gui\` |
| `{"ok":false,"error":"..."}` shown | PHP working, logic error | Check the error message — usually missing field |
| Blank page or 403 | Directory permissions | Add `Require all granted` to Directory block |
| http://localhost broken | Missing localhost VirtualHost | Add the localhost `<VirtualHost>` block shown in the conf file |
| PHP not parsed (shown as text) | PHP handler not active | WAMP tray → PHP → enable the PHP version |
| Can't write users.csv | File permissions | Give `IUSR` or `Everyone` Modify on the `data\` folder |

---

## Quick file path reference

| What | Windows path |
|---|---|
| VirtualHost config | `C:\wamp64\bin\apache\apache2.4.XX\conf\extra\httpd-vhosts.conf` |
| httpd.conf | `C:\wamp64\bin\apache\apache2.4.XX\conf\httpd.conf` |
| hosts file | `C:\Windows\System32\drivers\etc\hosts` |
| WAMP error log | `C:\wamp64\logs\` |
| Apache error log for this site | `C:\wamp64\logs\thinkfast-error.log` |

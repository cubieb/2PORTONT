#!bin/sh

echo '' > /var/group
echo "admin:$1$$9rSWmaINP3I0oRPRGR6t31:0:0::/tmp:/bin/sh" > /var/passwd
/bin/smbd &

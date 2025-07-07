rm -rf prisma/database
npx prisma migrate dev --name initialize
mv database/app.db database/app.db.bak
mv prisma/database/app.db database/app.db
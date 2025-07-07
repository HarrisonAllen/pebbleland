import { PrismaClient } from '@prisma/client'
const prisma = new PrismaClient()
async function main() {
  const admin = await prisma.user.create({
    data: {
      accountID: '0',
      watchID: '0',
      accountInfo: {
        create: {
          password: 'password',
          email: 'email'
        }
      },
      playerInfo: {
        create: {
          username: 'admin'
        }
      }
    }
  });
  console.log("Created: ");
  console.log(admin);
  const users = await prisma.user.findMany();
  console.log("Verification: ")
  console.log(users);
}
main();
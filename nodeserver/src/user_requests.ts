import { PrismaClient } from '@prisma/client';
import * as auth from "./authentication"
import { prisma } from './prisma';
import * as errs from "./error_handling";

// res.status(200).send('Welcome to Pebble Park, ' + req.user.username);

export async function get_my_info(req: any, res: any) {
    const { username } = req.user;
    try {
        const user = await prisma.testUser.findUnique({
            where: {
                username: username,
            },
            select: {
                favoriteNumber: true,
            }
        });

        res.status(200).json({ user:user });
    } catch (error:any) {
        errs.unknown_user(res, username);
    }
}

export async function set_fave_num(req: any, res: any) {
    const { number } = req.body;
    const { username } = req.user;
    const int_number = parseInt(number);
    if (isNaN(int_number)) {
        return errs.generic_error(res, "Invalid number");
    }

    try {
        const updated_number = await prisma.testUser.update({
            where: {
                username: username,
            },
            data: {
                favoriteNumber: int_number,
            },
            select: {
                favoriteNumber: true,
            }
        });
        res.status(200).json({ number:updated_number.favoriteNumber});
    } catch (error:any) {
        errs.unknown_user(res, username);
    }
}

export async function get_users(req: any, res: any) {
    try {
        // const users = await prisma.testUser.findMany({select: {username: true}});
        const users = await prisma.testUser.findMany();
        res.status(200).json(users);
    } catch (error:any) {
        errs.generic_error(res, "Failed to get users");
    }
}
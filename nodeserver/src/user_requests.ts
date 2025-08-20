import { prisma } from './prisma';
import * as errs from "./error_handling";

// res.status(200).send('Welcome to Pebble Park, ' + req.user.username);

export async function get_my_info(req: any, res: any) {
    const { account_id, watch_id } = req.user_id;
    console.log(req.user_id);
    console.log("-> " + account_id + " " + watch_id);
    try {
        const user = await prisma.user.findUnique({ 
            where: { 
                userID: {
                    accountID: account_id,
                    watchID: watch_id 
                },
            },
            select: {
                playerInfo: true
            }
        });

        res.status(200).json({ user:user });
    } catch (error:any) {
        console.log(error);
        errs.unknown_user(res);
    }
}

/* TODO: remove once pattern is implemented for real user (player update)
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
        errs.unknown_user(res);
    }
}
*/

export async function get_users(req: any, res: any) {
    try {
        const users = await prisma.user.findMany({
            include: {
                accountInfo: false,
                playerInfo: true
            }
        });
        console.log(users);
        res.status(200).json(users);
    } catch (error:any) {
        console.log(error)
        errs.generic_error(res, "Failed to get users");
    }
}

export async function update_username(req: any, res: any) {
    const { account_id, watch_id } = req.user_id;
    const { username } = req.body;
    try {
        const this_user = await prisma.user.findUnique({ 
            where: { 
                userID: {
                    accountID: account_id,
                    watchID: watch_id 
                },
            },
            select: {
                playerInfo: {
                    select: {
                        username: true
                    }
                }
            }
        });

        if (this_user?.playerInfo?.username == username) {
            return res.status(200).json({"username_updated": false, "old_username": this_user?.playerInfo?.username, "new_username": username});
        }

        const user_with_name = await prisma.user.findFirst({ 
            where: { 
                playerInfo: {
                    username: username
                },
            }
        });

        if (user_with_name != null) {
            return res.status(200).json({"username_updated": false,  "old_username": this_user?.playerInfo?.username, "new_username": username});
        }

        const user = await prisma.user.update({
            where: {
                userID: {
                    accountID: account_id,
                    watchID: watch_id 
                }
            },
            data: {
                playerInfo: {
                    update: {
                        username: username
                    }
                }
            }
        });

        console.log("Updated username to " + username);
        res.status(200).json({"username_updated": true,  "old_username": this_user?.playerInfo?.username, "new_username": username});
    } catch (error:any) {
        console.log(error)
        errs.generic_error(res, "Failed to get users");
    }
}

export async function update_email(req: any, res: any) {
    const { account_id, watch_id } = req.user_id;
    const { email } = req.body;
    try {
        const user = await prisma.user.update({
            where: {
                userID: {
                    accountID: account_id,
                    watchID: watch_id 
                }
            },
            data: {
                accountInfo: {
                    update: {
                        email: email
                    }
                }
            }
        });

        console.log("Updated email to " + email);
        res.status(200).json({"email_updated": true, "email": email});
    } catch (error:any) {
        console.log(error)
        errs.generic_error(res, "Failed to get users");
    }
}
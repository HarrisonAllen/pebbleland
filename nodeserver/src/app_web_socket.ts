import { ERROR_CODES } from "./error_handling";
import { prisma } from "./prisma";

export class AppWebSocket {
    ws: WebSocket;
    authenticated: boolean = false;
    account_id: string = "";
    watch_id: string = "";
    username: string|undefined = undefined;
    x: number = 0;
    y: number = 0;
    dir: number = 0;
    hair_style: number = 0;
    shirt_style: number = 0;
    pants_style: number = 0;
    hair_color: number = 0;
    shirt_color: number = 0;
    pants_color: number = 0;
    shoes_color: number = 0;

    constructor(ws: WebSocket) {
        this.ws = ws;
    }

    get_player_location(): {[item: string] : string|number|undefined} {
        var data:{[item: string] : string|number|undefined} = {};
        data["x"] = this.x;
        data["y"] = this.y;
        data["dir"] = this.dir;
        return data
    }

    get_player_data(): {[item: string] : string|number|undefined} {
        var data:{[item: string] : string|number|undefined} = {};
        data["hair_style"] = this.hair_style;
        data["shirt_style"] = this.shirt_style;
        data["pants_style"] = this.pants_style;
        data["hair_color"] = this.hair_color;
        data["shirt_color"] = this.shirt_color;
        data["pants_color"] = this.pants_color;
        data["shoes_color"] = this.shoes_color;

        return data
    }

    set_player_location(data: {[item: string] : any}) {
        this.x = data["x"];
        this.y = data["y"];
        this.dir = data["dir"];
    }

    async set_player_data(data: {[item: string] : any}) {
        this.hair_style = data["hair_style"];
        this.shirt_style = data["shirt_style"];
        this.pants_style = data["pants_style"];
        this.hair_color = data["hair_color"];
        this.shirt_color = data["shirt_color"];
        this.pants_color = data["pants_color"];
        this.shoes_color = data["shoes_color"];

        const user = await prisma.user.update({
            where: {
                userID: {
                    accountID: this.account_id,
                    watchID: this.watch_id 
                }
            },
            data: {
                playerInfo: {
                    update: {
                        hairStyle: this.hair_style,
                        shirtStyle: this.shirt_style,
                        pantsStyle: this.pants_style,
                        hairColor: this.hair_color,
                        shirtColor: this.shirt_color,
                        pantsColor: this.pants_color,
                        shoesColor: this.shoes_color,
                    }
                }
            }
        });
    }
    
    async load_player_data() {
        const user = await prisma.user.findUnique({ 
            where: { 
                userID: {
                    accountID: this.account_id,
                    watchID: this.watch_id 
                }
            },
            select: {
                playerInfo: {
                    select: {
                        username: true,
                        status: true,
                        hairStyle: true,
                        shirtStyle: true,
                        pantsStyle: true,
                        hairColor: true,
                        shirtColor: true,
                        pantsColor: true,
                        shoesColor: true,
                    }
                }
            }
        });
        this.username = user.playerInfo.username;
        this.set_player_data({
            "hair_style": user.playerInfo.hairStyle,
            "shirt_style": user.playerInfo.shirtStyle,
            "pants_style": user.playerInfo.pantsStyle,
            "hair_color": user.playerInfo.hairColor,
            "shirt_color": user.playerInfo.shirtColor,
            "pants_color": user.playerInfo.pantsColor,
            "shoes_color": user.playerInfo.shoesColor,
        });
    }
};
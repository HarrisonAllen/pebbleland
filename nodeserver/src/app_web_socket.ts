export class AppWebSocket {
    ws: WebSocket;
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

    set_player_data(data: {[item: string] : any}) {
        this.hair_style = data["hair_style"];
        this.shirt_style = data["shirt_style"];
        this.pants_style = data["pants_style"];
        this.hair_color = data["hair_color"];
        this.shirt_color = data["shirt_color"];
        this.pants_color = data["pants_color"];
        this.shoes_color = data["shoes_color"];
    }
};
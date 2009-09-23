import mx.DateTime

def format_quantity(value, singular, plural=None):
    if value == 1:
        return "%s %s" % (str(value), singular)
    elif plural is None:
        return "%s %ss" % (str(value), singular)
    else:
        return "%s %s" % (str(value), plural)


def format_time_delta(delta, tooltip = False):
    if tooltip:
        if delta is None:
            return "-"
        else:
            return "%id:%.2ih:%.2im:%.2is" % (delta.day, delta.hour, delta.minute, delta.second)
    else:
        if delta is None:
            return "-"
        else:
            delta_part2 = None

            if delta.day != 0:
                delta_part1 = format_quantity(delta.day, "day")
                if delta.minute != 0:
                    delta_part2 = format_quantity(delta.minute, "hour")

            elif delta.hour != 0:
                delta_part1 = format_quantity(delta.hour, "hour")
                if delta.minute != 0:
                    delta_part2 = format_quantity(delta.minute, "minute")

            elif delta.minute != 0:
                delta_part1 = format_quantity(delta.minute, "minute")
                if delta.second != 0:
                    delta_part2 = format_quantity(int(delta.second), "second")

            elif delta.second != 0:
                delta_part1 = format_quantity(int(delta.second), "second")

            else:
                return "-"

            if delta_part2 is None:
                return delta_part1
            else:
                return "%s and %s" % (delta_part1, delta_part2)


def format_timestamp(time_in, tooltip = False):
    if tooltip:
        if time_in is None:
            return "unassigned"
        else:
            return time_in.strftime("%c")
    else:
        if time_in is None:
            return "-"
        else:
            now = mx.DateTime.now()
            delta = time_in - now
            past = (delta.seconds < 0)
            delta = abs(delta)
            time_part2 = None

            if delta.day == 0:
                time_part1 = time_in.strftime("%X")

                if delta.hour == 0:
                    time_part2 = format_quantity(delta.minute, "minute")
                else:
                    time_part2 = format_quantity(delta.hour, "hour")

            elif time_in.year == now.year:
                time_part1 = time_in.strftime("%b %d")

                if delta.day < 7:
                    time_part2 = format_quantity(delta.day, "day")
                else:
                    time_part2 = format_quantity(int(delta.day/7), "week")

            else:
                return delta.strftime("%x")

            if past:
                return "%s <span style=\"font-size: x-small;\">(%s ago)</span>" % (time_part1, time_part2)
            else:
                return "%s <span style=\"font-size: x-small;\">(in %s)</span>" % (time_part1, time_part2)


def format_pid(pid):
    if pid == -1:
        return "-"
    else:
        return str(pid)


def format_context(file, line_no):
    if line_no == -1:
        return file
    else:
        return "%s:%i" % (file, line_no)

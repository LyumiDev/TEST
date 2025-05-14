import discord
from discord.ext import commands, tasks
import json
import os
import random
import aiohttp

# Set up intents
intents = discord.Intents.default()
intents.messages = True
intents.guilds = True
intents.message_content = True

# Initialize the bot
bot = commands.Bot(command_prefix="!", intents=intents)


bot.remove_command("help")  # Remove the default help command


# Set up status options
status_options = [
    "Managing systems",
    "Proxying messages",
    "Organizing folders",
    "Handling proxies",
    "!pixelhelp for all commands",
    "Connecting systems",
    "Serving multiple servers"
]

# Update the bot's status every 60 seconds
@bot.event
async def on_ready():
    print(f"✅ Bot is online as {bot.user}")
    await update_status()  # Set status immediately on startup
    change_status.start()

@tasks.loop(seconds=60)
async def change_status():
    await update_status()

async def update_status():
    new_status = random.choice(status_options)
    await bot.change_presence(activity=discord.Game(name=new_status))

import os

# Use the bot token from the Replit secret
bot.run(os.environ["BOT_TOKEN"])



PROFILES_FILE = "global_profiles.json"

# Ensure the global profiles file exists
if not os.path.exists(PROFILES_FILE):
    with open(PROFILES_FILE, "w") as f:
        json.dump({}, f)

# Load profiles from file
def load_profiles():
    with open(PROFILES_FILE, "r") as f:
        try:
            return json.load(f)
        except json.JSONDecodeError:
            return {}

# Save profiles to file
def save_profiles(profiles):
    with open(PROFILES_FILE, "w") as f:
        json.dump(profiles, f, indent=4)

global_profiles = load_profiles()

CATEGORY_BLACKLIST_FILE = "category_blacklist.json"

# Ensure the category blacklist file exists
if not os.path.exists(CATEGORY_BLACKLIST_FILE):
    with open(CATEGORY_BLACKLIST_FILE, "w") as f:
        json.dump({}, f)

# Load the category blacklist from file
def load_category_blacklist():
    with open(CATEGORY_BLACKLIST_FILE, "r") as f:
        try:
            return json.load(f)
        except json.JSONDecodeError:
            return {}

# Save the category blacklist to file
def save_category_blacklist(blacklist):
    with open(CATEGORY_BLACKLIST_FILE, "w") as f:
        json.dump(blacklist, f, indent=4)

category_blacklist = load_category_blacklist()

BLACKLIST_FILE = "channel_blacklist.json"

# Ensure the blacklist file exists
if not os.path.exists(BLACKLIST_FILE):
    with open(BLACKLIST_FILE, "w") as f:
        json.dump({}, f)

# Load the blacklist from file
def load_blacklist():
    with open(BLACKLIST_FILE, "r") as f:
        try:
            return json.load(f)
        except json.JSONDecodeError:
            return {}

# Save the blacklist to file
def save_blacklist(blacklist):
    with open(BLACKLIST_FILE, "w") as f:
        json.dump(blacklist, f, indent=4)

channel_blacklist = load_blacklist()


# Backup profiles to a separate file for safety
def backup_profiles():
    with open(BACKUP_FILE, "w") as f:
        json.dump(profiles, f, indent=4)

# Remove the default help command to prevent conflicts
bot.remove_command("help")

@bot.event
async def on_ready():
    print(f"Bot is online as {bot.user}")





import discord
from discord.ext import commands
from discord.ui import View, Button

class ProfilePaginator(View):
    def __init__(self, user_id, system_name, pages):
        super().__init__(timeout=180)
        self.user_id = user_id
        self.system_name = system_name
        self.pages = pages
        self.current_page = 0
        self.total_pages = len(pages)

        # Disable buttons if there is only one page
        if self.total_pages == 1:
            self.disable_buttons()

    def disable_buttons(self):
        for child in self.children:
            if isinstance(child, Button):
                child.disabled = True

    async def update_message(self, interaction):
        embed = discord.Embed(
            title=f"🗂️ Profiles for {self.system_name} (Page {self.current_page + 1}/{self.total_pages})",
            description="\n".join(self.pages[self.current_page]),
            color=0x8A2BE2  # Default to purple
        )
        embed.set_footer(text=f"User ID: {self.user_id}")
        await interaction.response.edit_message(embed=embed, view=self)

    @discord.ui.button(label="⬅️ Previous", style=discord.ButtonStyle.secondary)
    async def previous_page(self, interaction, button):
        self.current_page -= 1
        if self.current_page < 0:
            self.current_page = self.total_pages - 1  # Wrap around to the last page
        await self.update_message(interaction)

    @discord.ui.button(label="➡️ Next", style=discord.ButtonStyle.secondary)
    async def next_page(self, interaction, button):
        self.current_page += 1
        if self.current_page >= self.total_pages:
            self.current_page = 0  # Wrap around to the first page
        await self.update_message(interaction)





# Delete a profile
@bot.command()
async def delete(ctx, name: str):
    if name not in profiles:
        await ctx.send(f"Profile '{name}' does not exist.")
        return
    del profiles[name]
    save_profiles()
    backup_profiles()
    await ctx.send(f"Profile '{name}' has been deleted successfully.")

# Add an alias to a profile
@bot.command()
async def alias(ctx, name: str, *, alias: str):
    if name not in profiles:
        await ctx.send(f"Profile '{name}' does not exist.")
        return
    profiles[name].setdefault("aliases", []).append(alias)
    save_profiles()
    backup_profiles()
    await ctx.send(f"Alias '{alias}' added to profile '{name}' successfully!")

# Remove an alias from a profile
@bot.command()
async def remove_alias(ctx, name: str, *, alias: str):
    if name not in profiles:
        await ctx.send(f"Profile '{name}' does not exist.")
        return
    if alias not in profiles[name].get("aliases", []):
        await ctx.send(f"Alias '{alias}' does not exist for profile '{name}'.")
        return
    profiles[name]["aliases"].remove(alias)
    save_profiles()
    backup_profiles()
    await ctx.send(f"Alias '{alias}' removed from profile '{name}' successfully!")

# Backup all profiles manually
@bot.command()
async def backup(ctx):
    backup_profiles()
    await ctx.send("Profiles have been backed up successfully!")




import aiohttp

# Detect and resend proxies as webhooks, while respecting the blacklist and proxy avatars
@bot.event
async def on_message(message):
    # Ignore messages from bots (including itself)
    if message.author.bot:
        return

    # Determine if the message is in a guild or a DM
    if message.guild:
        guild_id = str(message.guild.id)
        category_id = message.channel.category_id

        # Check if the message is in a blacklisted category
        if guild_id in category_blacklist and category_id in category_blacklist[guild_id]:
            return  # Ignore messages in blacklisted categories

        # Check if the message is in a blacklisted channel
        if guild_id in channel_blacklist and message.channel.id in channel_blacklist[guild_id]:
            return  # Ignore messages in blacklisted channels

    user_id = str(message.author.id)
    user_profiles = global_profiles.get(user_id, {}).get("alters", {})

    # Check each profile for a matching proxy
    for name, profile in user_profiles.items():
        proxy = profile.get("proxy")
        displayname = profile.get("displayname", name)

        # Use the proxy avatar if it exists, otherwise use the main avatar
        proxy_avatar = profile.get("proxy_avatar") or profile.get("avatar")

        # If the profile has a proxy set, check for it
        if proxy and message.content.startswith(proxy):
            # Remove the proxy from the message
            clean_message = message.content[len(proxy):].strip()

            # Create a webhook with the alter's display name (only for guilds)
            if message.guild:
                webhook = await message.channel.create_webhook(name=displayname)

                # Use the proxy avatar if it exists
                if proxy_avatar:
                    try:
                        async with aiohttp.ClientSession() as session:
                            async with session.get(proxy_avatar) as response:
                                if response.status == 200:
                                    avatar_bytes = await response.read()
                                    # Update the webhook to use the proxy avatar
                                    await webhook.edit(avatar=avatar_bytes)
                                else:
                                    print(f"Failed to fetch avatar for {displayname}: {response.status}")
                    except Exception as e:
                        print(f"Error setting avatar for {displayname}: {e}")

                # Send the proxied message
                await webhook.send(
                    content=clean_message,
                    username=displayname,
                    allowed_mentions=discord.AllowedMentions.none()
                )

                # Delete the original message to prevent double posting
                try:
                    await message.delete()
                except discord.Forbidden:
                    print(f"⚠️ Missing permissions to delete message in {message.channel.name}")

                # Delete the webhook after sending the message
                await webhook.delete()

            else:
                # Send the message directly in DMs without using a webhook
                await message.channel.send(
                    content=clean_message,
                    username=displayname,
                    allowed_mentions=discord.AllowedMentions.none()
                )

            return  # Stop after the first matching proxy is found

    # Process other bot commands normally
    await bot.process_commands(message)


import discord
import re

# Show system info for the current user with full Markdown support, including links
@bot.command(name="system")
async def system(ctx):
    user_id = str(ctx.author.id)
    user_data = global_profiles.get(user_id, {})
    system_info = user_data.get("system", {})

    # Check if the system exists
    if not system_info:
        await ctx.send("❌ You don't have a system set up yet. Use `!create_system` to create one.")
        return

    # Extract system info
    system_name = system_info.get("name", "Unnamed System")
    system_pronouns = system_info.get("pronouns", "Not set")
    system_description = system_info.get("description", "No description provided.")
    system_avatar = system_info.get("avatar", None)
    system_banner = system_info.get("banner", None)
    system_color = system_info.get("color", 0x8A2BE2)  # Default to purple

    # Preserve Markdown links without escaping them
    def preserve_links(text):
        # Preserve links in [text](link) format
        return re.sub(
            r"(?<!\\)\[([^\]]+)\]\((https?://[^\s)]+)\)",
            r"[\1](\2)",
            text
        )

    # Process the description for markdown and links
    system_description = preserve_links(system_description)

    # Create the system info embed
    embed = discord.Embed(
        title=system_name,
        description=(
            f"**System Name:** {system_name}\n"
            f"**Pronouns:** {system_pronouns}\n"
            f"**Description:** {system_description}\n\n"
            f"||Linked Discord Account: {ctx.author.mention}||"
        ),
        color=system_color
    )

    # Set system avatar as thumbnail if it exists
    if system_avatar:
        embed.set_thumbnail(url=system_avatar)

    # Set system banner as main image if it exists
    if system_banner:
        embed.set_image(url=system_banner)

    # Set a footer with the user ID for reference
    embed.set_footer(text=f"User ID: {user_id}")

    await ctx.send(embed=embed)


























import os

# Use the bot token from the Replit secret
my_secret = os.environ['BOT_TOKEN']


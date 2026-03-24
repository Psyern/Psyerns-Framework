class PF_DiscordEmbedField
{
	string name;
	string value;
	bool inline;

	void PF_DiscordEmbedField(string fieldName, string fieldValue, bool isInline = false)
	{
		name = fieldName;
		value = fieldValue;
		inline = isInline;
	}
}

class PF_DiscordEmbedAuthor
{
	string name;
	string icon_url;

	void PF_DiscordEmbedAuthor(string authorName, string iconUrl = "")
	{
		name = authorName;
		icon_url = iconUrl;
	}
}

class PF_DiscordEmbed
{
	string title;
	string description;
	int color;
	string timestamp;
	ref PF_DiscordEmbedAuthor author;
	ref array<ref PF_DiscordEmbedField> fields;

	void PF_DiscordEmbed()
	{
		color = 3447003; // blue default
		fields = new array<ref PF_DiscordEmbedField>();
	}

	PF_DiscordEmbed SetTitle(string t)
	{
		title = t;
		return this;
	}

	PF_DiscordEmbed SetDescription(string desc)
	{
		description = desc;
		return this;
	}

	PF_DiscordEmbed SetColor(int c)
	{
		color = c;
		return this;
	}

	PF_DiscordEmbed SetTimestamp(string ts)
	{
		timestamp = ts;
		return this;
	}

	PF_DiscordEmbed SetAuthor(string name, string iconUrl = "")
	{
		author = new PF_DiscordEmbedAuthor(name, iconUrl);
		return this;
	}

	PF_DiscordEmbed AddField(string name, string value, bool isInline = false)
	{
		fields.Insert(new PF_DiscordEmbedField(name, value, isInline));
		return this;
	}

	string Serialize()
	{
		PF_JsonBuilder builder = PF_JsonBuilder.Begin();
		builder.Add("title", title);
		builder.Add("description", description);
		builder.AddInt("color", color);

		if (timestamp != "")
			builder.Add("timestamp", timestamp);

		if (author)
		{
			PF_JsonBuilder authorObj = PF_JsonBuilder.Begin();
			authorObj.Add("name", author.name);
			if (author.icon_url != "")
				authorObj.Add("icon_url", author.icon_url);

			builder.AddObject("author", authorObj);
		}

		if (fields.Count() > 0)
		{
			string fieldsArr = "[";
			for (int i = 0; i < fields.Count(); i++)
			{
				if (i > 0)
					fieldsArr += ",";

				PF_JsonBuilder fb = PF_JsonBuilder.Begin();
				fb.Add("name", fields[i].name);
				fb.Add("value", fields[i].value);
				fb.AddBool("inline", fields[i].inline);
				fieldsArr += fb.Build();
			}
			fieldsArr += "]";
			builder.AddRaw("fields", fieldsArr);
		}

		return builder.Build();
	}
}

class PF_DiscordPayload
{
	string username;
	string avatar_url;
	string content;
	ref array<ref PF_DiscordEmbed> embeds;

	void PF_DiscordPayload()
	{
		username = "Psyerns Framework";
		avatar_url = "";
		embeds = new array<ref PF_DiscordEmbed>();
	}

	PF_DiscordEmbed CreateEmbed()
	{
		PF_DiscordEmbed embed = new PF_DiscordEmbed();
		embeds.Insert(embed);
		return embed;
	}

	string Serialize()
	{
		PF_JsonBuilder builder = PF_JsonBuilder.Begin();

		if (username != "")
			builder.Add("username", username);

		if (avatar_url != "")
			builder.Add("avatar_url", avatar_url);

		if (content != "")
			builder.Add("content", content);

		if (embeds.Count() > 0)
		{
			string embedsArr = "[";
			for (int i = 0; i < embeds.Count(); i++)
			{
				if (i > 0)
					embedsArr += ",";

				embedsArr += embeds[i].Serialize();
			}
			embedsArr += "]";
			builder.AddRaw("embeds", embedsArr);
		}

		return builder.Build();
	}
}
